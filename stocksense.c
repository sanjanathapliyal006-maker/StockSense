/*
 * StockSense - Smart Inventory, Simple C
 * Compile: gcc stocksense.c -o stocksense     Run: ./stocksense [--help | --version | --demo]
 * Logins : admin/admin123 (full access)       staff/staff123 (view, sell, reports)
 * Tags [U1.x] [U2.x] [U3.x] = syllabus chapter that the code belongs to.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>                          /* only for today's date and timestamps */
#include <limits.h>

/* ===== [U3.3] PREPROCESSOR DIRECTIVES AND MACROS ===== */
#define PROJECT_NAME "StockSense"
#define VERSION      "2.0"
#ifndef MAX_CART
#define MAX_CART     20
#endif
#define START_CAP    10
#define GST_RATE     0.05f
#define SOON_DAYS    7
#define ROWS         8
#define DATA_FILE    "inventory.dat"       /* binary file */
#define LOG_FILE     "log.txt"             /* text file   */
#define CALC_GST(x)  ((x) * GST_RATE)
#define MIN_OF(a, b) (((a) < (b)) ? (a) : (b))
#define NEED(p)      if (!(perms & (p))) { printf("Access denied for your role.\n"); break; }
#ifdef DEBUG                               /* compile with -DDEBUG to see debug prints */
#define DEBUG_LOG(m) printf("[DEBUG] %s\n", m)
#else
#define DEBUG_LOG(m)
#endif
/* [U1.2] BITWISE OPERATORS: item flags and role permissions are bit masks */
#define F_PERISHABLE 0x01
#define F_TAXABLE    0x02
#define P_VIEW       0x01
#define P_SELL       0x02
#define P_REP        0x04
#define P_ADMIN      0x08
#define PERMS_ADMIN  (P_VIEW | P_SELL | P_REP | P_ADMIN)
#define PERMS_STAFF  (P_VIEW | P_SELL | P_REP)

/* ===== [U3.2] STRUCTURES, NESTED STRUCTURE, UNION ===== */
struct Date { int d, m, y; };                           /* y == 0 means "no expiry" */
union Measure { float kg; float litre; int pieces; };   /* only one member is valid at a time */
struct Item {
    int id;
    char name[50], category[30];                        /* [U2.2] character arrays */
    int quantity;
    float price;
    int reorder, sold;
    struct Date expiry;                                 /* nested structure */
    unsigned char flags;
    char unit;                                          /* 'W' kg, 'V' litre, 'P' pieces = which union member is valid */
    union Measure size;
    int week[7];                                        /* units sold per weekday, Sun..Sat */
};
struct CartLine { int idx, qty, taxable; char name[50]; float price; };
/* ===== [U1.1] GLOBAL VARIABLES (global scope, live for the whole program) ===== */
struct Item *items = NULL;                              /* [U2.3] dynamic array */
int count = 0, capacity = 0, billNo = 1000, perms = 0;
char user[30] = "";
const char *dayName[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
/* ===== INPUT HELPERS [U1.1 formatted I/O] ===== */
void clearLine(void) { int c; while ((c = getchar()) != '\n' && c != EOF); }
double readNum(const char *p) {
    double x; int r;
    printf("%s", p);
    while ((r = scanf("%lf", &x)) != 1) {
        if (r == EOF) { printf("\nInput ended. Exiting.\n"); exit(0); }
        clearLine(); printf("Invalid input. Enter a number: ");
    }
    clearLine();
    return x;
}
int readInt(const char *p) {
    double x;
    for (;;) {
        x = readNum(p);
        if (x >= INT_MIN && x <= INT_MAX && x == (int)x) return (int)x;
        printf("Invalid input. Enter a whole number in range: ");
    }
}
float readFloat(const char *p) { return (float)readNum(p); }
void readString(const char *p, char *buf, int size) {
    int c;
    printf("%s", p);
    if (fgets(buf, size, stdin) == NULL) { printf("\nInput ended. Exiting.\n"); exit(0); }
    if (strchr(buf, '\n') == NULL) while ((c = getchar()) != '\n' && c != EOF);
    buf[strcspn(buf, "\n")] = '\0';
}
int confirm(const char *msg) {
    char b[10];
    printf("%s (y/n): ", msg);
    return fgets(b, sizeof(b), stdin) != NULL && (b[0] == 'y' || b[0] == 'Y');
}
/* ===== [U2.2] MANUAL STRING MANIPULATION: partial, case-insensitive search without strstr ===== */
int myStrLen(const char *s) { int n = 0; while (s[n] != '\0') n++; return n; }
char myLower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }    /* [U1.3] conditional operator */
int containsIgnoreCase(const char *hay, const char *key) {
    int hl = myStrLen(hay), kl = myStrLen(key);
    for (int i = 0; kl > 0 && i + kl <= hl; i++) {
        int j = 0;
        while (j < kl && myLower(hay[i + j]) == myLower(key[j])) j++;
        if (j == kl) return 1;
    }
    return 0;
}
/* ===== DATE HELPERS (arrays and loops, no library needed) ===== */
int isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
int daysIn(int m, int y) {
    int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : dim[m - 1];
}
int validDate(struct Date t) {
    return t.y >= 2000 && t.y <= 2100 && t.m >= 1 && t.m <= 12 && t.d >= 1 && t.d <= daysIn(t.m, t.y);
}
long dayNumber(struct Date t) {                         /* days since 01/01/2000 */
    long n = t.d;
    for (int y = 2000; y < t.y; y++) n += isLeap(y) ? 366 : 365;
    for (int m = 1; m < t.m; m++) n += daysIn(m, t.y);
    return n;
}
struct Date today(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    struct Date d = {t->tm_mday, t->tm_mon + 1, t->tm_year + 1900};
    return d;
}
struct Date addDays(struct Date t, int n) {
    while (n > 0) { if (++t.d > daysIn(t.m, t.y)) { t.d = 1; if (++t.m > 12) { t.m = 1; t.y++; } } n--; }
    while (n < 0) { if (--t.d < 1) { if (--t.m < 1) { t.m = 12; t.y--; } t.d = daysIn(t.m, t.y); } n++; }
    return t;
}
int hasExpiry(const struct Item *it) { return it->expiry.y != 0; }
long daysLeft(struct Date e) { return dayNumber(e) - dayNumber(today()); }      /* negative = expired */
void dateStr(struct Date t, char *o) { if (t.y == 0) strcpy(o, "N/A"); else sprintf(o, "%02d/%02d/%04d", t.d, t.m, t.y); }
struct Date readExpiry(void) {
    struct Date t;
    do {
        t.d = readInt("  Expiry day   : ");
        t.m = readInt("  Expiry month : ");
        t.y = readInt("  Expiry year (yyyy): ");
        if (!validDate(t)) printf("Invalid date, try again.\n");
    } while (!validDate(t));
    return t;
}
/* ===== FORMAT HELPERS ===== */
const char *statusOf(const struct Item *it) {           /* nested conditional operator */
    return (hasExpiry(it) && daysLeft(it->expiry) < 0) ? "EXPIRED" : (it->quantity <= it->reorder) ? "LOW STOCK" : "OK";
}
void flagStr(unsigned char f, char *o) {                /* [U1.2] bitwise AND tests a bit */
    o[0] = (f & F_PERISHABLE) ? 'P' : '-';
    o[1] = (f & F_TAXABLE) ? 'T' : '-';
    o[2] = '\0';
}
void sizeStr(const struct Item *it, char *o) {
    switch (it->unit) {                                 /* unit tells which union member to read */
        case 'W': sprintf(o, "%.2fkg", it->size.kg); break;
        case 'V': sprintf(o, "%.2fL", it->size.litre); break;
        case 'P': sprintf(o, "%dpcs", it->size.pieces); break;
        default: strcpy(o, "-");
    }
}
void timestamp(char *buf, size_t size) {
    time_t t = time(NULL);
    snprintf(buf, size, "%s", ctime(&t));
    buf[strcspn(buf, "\n")] = '\0';
}
void logAction(const char *msg) {                       /* [U3.3] text file, append mode */
    FILE *fp = fopen(LOG_FILE, "a");
    char ts[40];
    if (fp == NULL) return;
    timestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] (%s) %s\n", ts, user[0] ? user : "system", msg);
    fclose(fp);
}
/* ===== [U2.3] DYNAMIC MEMORY: malloc, realloc, pointer-to-pointer ===== */
void initInventory(void) {
    items = (struct Item *)malloc(START_CAP * sizeof(struct Item));
    if (items == NULL) { printf("Fatal: not enough memory.\n"); exit(1); }
    capacity = START_CAP;
}
int growInventory(struct Item **arr, int *cap) {        /* ** so the caller's own pointer can change */
    struct Item *tmp = (struct Item *)realloc(*arr, (*cap * 2) * sizeof(struct Item));
    if (tmp == NULL) { printf("Memory error: cannot expand inventory.\n"); return 0; }
    *arr = tmp;
    *cap *= 2;
    DEBUG_LOG("inventory memory doubled");
    return 1;
}
struct Item *cloneItems(void) {                         /* malloc'd copy; caller must free it */
    struct Item *c = count ? (struct Item *)malloc(count * sizeof(struct Item)) : NULL;
    if (c != NULL) memcpy(c, items, count * sizeof(struct Item));
    return c;
}
struct Item *findById(int id) {                         /* [U2.3] pointer arithmetic; NULL = not found */
    for (struct Item *p = items; p < items + count; p++) if (p->id == id) return p;
    return NULL;
}
/* ===== [U3.3] BINARY FILES ===== */
void saveData(void) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (fp == NULL) { printf("Error: could not save data!\n"); return; }
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(&billNo, sizeof(int), 1, fp);
    fwrite(items, sizeof(struct Item), count, fp);
    fclose(fp);
}
void loadData(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    int n = 0;
    if (fp == NULL) return;                             /* first run: no file yet */
    if (fread(&n, sizeof(int), 1, fp) != 1 || fread(&billNo, sizeof(int), 1, fp) != 1 || n < 0 || n > 100000) goto bad_file;
    while (capacity < n) if (!growInventory(&items, &capacity)) goto bad_file;
    if (fread(items, sizeof(struct Item), n, fp) != (size_t)n) goto bad_file;
    count = n;
    fclose(fp);
    return;
bad_file:                                               /* [U1.4] goto used for error handling */
    printf("Warning: data file unreadable. Starting empty.\n");
    count = 0;
    fclose(fp);
}
/* ===== LOGIN [U1.2 logical operators] ===== */
int login(void) {
    char u[30], p[30];
    for (int attempt = 1; attempt <= 3; attempt++) {
        printf("\n--- LOGIN (attempt %d/3) ---\n", attempt);
        readString("Username: ", u, sizeof(u));
        readString("Password: ", p, sizeof(p));
        if (strcmp(u, "admin") == 0 && strcmp(p, "admin123") == 0) perms = PERMS_ADMIN;
        else if (strcmp(u, "staff") == 0 && strcmp(p, "staff123") == 0) perms = PERMS_STAFF;
        if (perms) { strcpy(user, u); logAction("LOGIN success"); return 1; }
        printf("Wrong username or password.\n");
    }
    return 0;
}
/* ===== DISPLAY AND ALERTS ===== */
void printTable(const struct Item *arr, int n) {
    if (n == 0) { printf("\nNo items to show.\n"); return; }
    printf("\n%-4s %-18s %-10s %-5s %-8s %-9s %-3s %-10s %s\n", "ID", "Name", "Category", "Qty", "Price", "Size", "Flg", "Expiry", "Status");
    puts("-------------------------------------------------------------------------");
    for (const struct Item *p = arr; p < arr + n; p++) {                        /* pointer arithmetic */
        char ds[16], ss[16], fs[8];
        dateStr(p->expiry, ds); sizeStr(p, ss); flagStr(p->flags, fs);
        printf("%-4d %-18.18s %-10.10s %-5d %-8.2f %-9s %-3s %-10s %s\n",
               p->id, p->name, p->category, p->quantity, p->price, ss, fs, ds, statusOf(p));
    }
    printf("Flags: P = Perishable, T = Taxable (GST)\n");
}
void alerts(void) {
    int low = 0, soon = 0;
    for (int i = 0; i < count; i++)
        if (items[i].quantity <= items[i].reorder) {
            if (!low++) printf("\n\a*** LOW STOCK ALERT ***\n");                /* \a = beep */
            printf("  !\t%s (ID %d): only %d left (reorder level %d)\n", items[i].name, items[i].id, items[i].quantity, items[i].reorder);
        }
    for (int i = 0; i < count; i++) {
        long d = hasExpiry(&items[i]) ? daysLeft(items[i].expiry) : 9999;
        if (d > SOON_DAYS) continue;
        if (!soon++) printf("\n*** EXPIRY ALERT (expired or within %d days) ***\n", SOON_DAYS);
        if (d < 0) printf("  X\t%s (ID %d): EXPIRED %ld days ago\n", items[i].name, items[i].id, -d);
        else printf("  !\t%s (ID %d): expires in %ld days\n", items[i].name, items[i].id, d);
    }
    if (!low && !soon) printf("\nNo low stock or expiry alerts.\n");
}
/* ===== CRUD ===== */
void addItem(void) {
    struct Item it;
    memset(&it, 0, sizeof(it));
    if (count >= capacity && !growInventory(&items, &capacity)) return;
    it.id = readInt("Enter Item ID: ");
    if (it.id <= 0 || findById(it.id) != NULL) { printf("ID must be positive and unique.\n"); return; }
    readString("Enter Item Name: ", it.name, sizeof(it.name));
    if (strlen(it.name) == 0) { printf("Name cannot be empty.\n"); return; }
    readString("Enter Category: ", it.category, sizeof(it.category));
    if (strlen(it.category) == 0) strcpy(it.category, "General");
    it.quantity = readInt("Enter Quantity: ");
    it.price = readFloat("Enter Price: ");
    it.reorder = readInt("Enter Reorder Level: ");
    if (it.quantity < 0 || it.price <= 0 || it.reorder < 0) { printf("Invalid values. Cancelled.\n"); return; }
    printf("Unit: 1. Weight (kg)  2. Volume (litre)  3. Pieces per pack\n");
    switch (readInt("Choice: ")) {
        case 1: it.unit = 'W'; it.size.kg = readFloat("Weight per unit (kg): "); break;
        case 2: it.unit = 'V'; it.size.litre = readFloat("Volume per unit (L): "); break;
        case 3: it.unit = 'P'; it.size.pieces = readInt("Pieces per pack: "); break;
        default: printf("Invalid unit.\n"); return;
    }
    if (confirm("Perishable (has expiry date)?")) it.flags |= F_PERISHABLE;     /* |= sets a bit */
    if (confirm("GST applicable (taxable)?")) it.flags |= F_TAXABLE;
    if (it.flags & F_PERISHABLE) it.expiry = readExpiry();
    items[count++] = it;
    saveData();
    { char m[160]; snprintf(m, sizeof(m), "ADDED item %d (%s) qty %d", it.id, it.name, it.quantity); logAction(m); }
    printf("Item \"%s\" added successfully!\n", it.name);
}
void searchItem(void) {
    int n = 0, ch;
    struct Item *res;
    printf("Search by: 1. ID   2. Name (partial, any case)\n");
    ch = readInt("Choice: ");
    if ((ch != 1 && ch != 2) || count == 0) { printf("Invalid choice or empty inventory.\n"); return; }
    res = (struct Item *)calloc(count, sizeof(struct Item));                    /* calloc: zero-filled memory */
    if (res == NULL) { printf("Memory error.\n"); return; }
    if (ch == 1) {
        struct Item *p = findById(readInt("Enter ID: "));
        if (p != NULL) res[n++] = *p;
    } else {
        char key[50];
        readString("Enter name or part of it: ", key, sizeof(key));
        for (int i = 0; i < count; i++) if (containsIgnoreCase(items[i].name, key)) res[n++] = items[i];
    }
    if (n == 0) printf("Item not found.\n"); else printTable(res, n);
    free(res);
    res = NULL;                                                                 /* avoid dangling pointer */
}
void updateItem(void) {
    struct Item *p = findById(readInt("Enter ID of item to update: "));
    int q; float pr;
    if (p == NULL) { printf("Item not found.\n"); return; }
    printf("Current: %s | Qty %d | Price %.2f\n", p->name, p->quantity, p->price);
    q = readInt("New Quantity: ");
    pr = readFloat("New Price: ");
    if (q < 0 || pr <= 0 || !confirm("Save these changes?")) { printf("Update cancelled.\n"); return; }
    p->quantity = q; p->price = pr;
    saveData();
    { char m[160]; snprintf(m, sizeof(m), "UPDATED item %d qty %d price %.2f", p->id, q, pr); logAction(m); }
    printf("Item updated.\n");
}
void deleteItem(void) {
    struct Item *p = findById(readInt("Enter ID of item to delete: "));
    char m[160];
    if (p == NULL) { printf("Item not found.\n"); return; }
    printf("You are about to delete: %s (ID %d)\n", p->name, p->id);
    if (!confirm("Are you sure?")) { printf("Delete cancelled.\n"); return; }
    snprintf(m, sizeof(m), "DELETED item %d (%s)", p->id, p->name);
    for (int i = (int)(p - items); i < count - 1; i++) items[i] = items[i + 1]; /* shift left to fill the gap */
    count--;
    saveData();
    logAction(m);
    printf("Item deleted.\n");
}
/* ===== [U3.1] RECURSION ===== */
float totalValueRec(const struct Item *a, int n) {
    if (n == 0) return 0;                                                       /* base case */
    return a[n - 1].quantity * a[n - 1].price + totalValueRec(a, n - 1);
}
int binarySearchRec(const struct Item *a, int lo, int hi, int id) {
    int mid;
    if (lo > hi) return -1;                                                     /* base case: not found */
    mid = lo + (hi - lo) / 2;
    if (a[mid].id == id) return mid;
    return (id < a[mid].id) ? binarySearchRec(a, lo, mid - 1, id) : binarySearchRec(a, mid + 1, hi, id);
}
/* ===== [U2.1] SORTING AND SEARCHING ===== */
int outOfOrder(const struct Item *a, const struct Item *b, int key) {
    switch (key) {
        case 1:  return strcmp(a->name, b->name) > 0;
        case 2:  return a->price > b->price;
        case 3:  return a->quantity > b->quantity;
        case 5:  return a->sold < b->sold;                                      /* highest sold first */
        default: return a->id > b->id;
    }
}
void bubbleSort(struct Item *a, int n, int key) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++)
            if (outOfOrder(&a[j], &a[j + 1], key)) { struct Item t = a[j]; a[j] = a[j + 1]; a[j + 1] = t; swapped = 1; }
        if (!swapped) break;
    }
}
void sortAndDisplay(void) {
    struct Item *c;
    int ch;
    printf("Sort by: 1. Name  2. Price (low-high)  3. Quantity (low-high)\n");
    ch = readInt("Choice: ");
    if (ch < 1 || ch > 3 || (c = cloneItems()) == NULL) { printf("Invalid choice or no items.\n"); return; }
    bubbleSort(c, count, ch);
    printTable(c, count);
    free(c);
}
void fastSearch(void) {
    int id = readInt("Enter ID (binary search): "), pos;
    struct Item *c = cloneItems();
    if (c == NULL) { printf("No items.\n"); return; }
    bubbleSort(c, count, 4);                                                    /* binary search needs sorted data */
    pos = binarySearchRec(c, 0, count - 1, id);
    if (pos < 0) printf("Item not found.\n"); else printTable(&c[pos], 1);
    free(c);
}
/* ===== BILLING ===== */
void printBill(FILE *out, int no, const char *ts, const struct CartLine *c, int n, float sub, float gst) {
    fprintf(out, "\n=============== %s ===============\nBill No : %d\nDate    : %s\nCashier : %s\n", PROJECT_NAME, no, ts, user);
    fprintf(out, "------------------------------------------------\n%-4s %-20s %-5s %-8s %-9s\n", "No", "Item", "Qty", "Rate", "Amount");
    for (int i = 0; i < n; i++)
        fprintf(out, "%-4d %-20.20s %-5d %-8.2f %-9.2f%s\n", i + 1, c[i].name, c[i].qty, c[i].price, c[i].qty * c[i].price, c[i].taxable ? " *" : "");
    fprintf(out, "------------------------------------------------\nSubtotal          : %10.2f\n", sub);
    fprintf(out, "GST (%.0f%% on *)   : %10.2f\nGRAND TOTAL       : %10.2f\n", GST_RATE * 100, gst, sub + gst);
    fprintf(out, "================================================\n        Thank you! Visit again.\n");
}
void billing(void) {
    static int sessionBills = 0;                                                /* [U1.1] static: value survives between calls */
    struct CartLine *cart = (struct CartLine *)calloc(MAX_CART, sizeof(struct CartLine));
    int n = 0, i, wd;
    float sub = 0, gst = 0;
    char ts[40], fname[40], msg[160];
    time_t now;
    FILE *fp;
    if (cart == NULL) { printf("Memory error.\n"); return; }
    printf("\n--- NEW BILL --- (Item ID 0 to finish)\n");
    while (n < MAX_CART) {
        int id = readInt("Item ID (0 = done): "), qty, pos = -1, inCart = 0;
        struct Item *p;
        if (id == 0) break;
        if ((p = findById(id)) == NULL) { printf("Item not found.\n"); continue; }
        if (hasExpiry(p) && daysLeft(p->expiry) < 0) { printf("%s is EXPIRED. Cannot sell.\n", p->name); continue; }
        for (i = 0; i < n; i++) if (cart[i].idx == (int)(p - items)) { pos = i; inCart = cart[i].qty; break; }
        printf("%s | Price %.2f | Available %d\n", p->name, p->price, p->quantity - inCart);
        qty = readInt("Quantity: ");
        if (qty <= 0 || qty + inCart > p->quantity) { printf("Invalid quantity or not enough stock!\n"); continue; }
        if (pos >= 0) cart[pos].qty += qty;                                     /* same item again: just add quantity */
        else {
            cart[n].idx = (int)(p - items); cart[n].qty = qty; cart[n].price = p->price;
            cart[n].taxable = (p->flags & F_TAXABLE) ? 1 : 0;
            strcpy(cart[n++].name, p->name);
        }
        printf("Added to cart.\n");
    }
    if (n == 0) { printf("Cart empty. Bill cancelled.\n"); goto done; }
    for (i = 0; i < n; i++) {
        float line = cart[i].qty * cart[i].price;
        sub += line;
        if (cart[i].taxable) gst += CALC_GST(line);                             /* GST only on taxable items */
    }
    timestamp(ts, sizeof(ts));
    printBill(stdout, billNo + 1, ts, cart, n, sub, gst);
    if (!confirm("Confirm sale?")) { printf("Sale cancelled. Stock unchanged.\n"); goto done; }
    billNo++; sessionBills++;
    now = time(NULL);
    wd = localtime(&now)->tm_wday;                                              /* 0 = Sunday */
    for (i = 0; i < n; i++) {                                                   /* stock changes only after confirmation */
        items[cart[i].idx].quantity -= cart[i].qty;
        items[cart[i].idx].sold += cart[i].qty;
        items[cart[i].idx].week[wd] += cart[i].qty;
    }
    saveData();
    snprintf(fname, sizeof(fname), "bill_%d.txt", billNo);
    if ((fp = fopen(fname, "w")) != NULL) {                                     /* text file, write mode */
        printBill(fp, billNo, ts, cart, n, sub, gst);
        fclose(fp);
        printf("Bill saved as %s\n", fname);
    }
    snprintf(msg, sizeof(msg), "SOLD bill %d: %d item type(s), total %.2f", billNo, n, sub + gst);
    logAction(msg);
    printf("Bills made this session: %d\n", sessionBills);
    for (i = 0; i < n; i++)
        if (items[cart[i].idx].quantity <= items[cart[i].idx].reorder)
            printf("WARNING: %s is low on stock (%d left)!\n", items[cart[i].idx].name, items[cart[i].idx].quantity);
done:
    free(cart);                                                                 /* freed on every exit path */
    cart = NULL;
}
/* ===== REPORTS ===== */
void salesReports(void) {
    struct Item *c = cloneItems();
    long total = 0;
    int shown = 0, dead = 0;
    if (c == NULL) { printf("No items.\n"); return; }
    bubbleSort(c, count, 5);
    for (int i = 0; i < count; i++) total += c[i].sold;
    printf("\n*** TOP SELLING ITEMS ***\n");
    for (int i = 0; i < count && shown < 5 && c[i].sold > 0; i++, shown++)
        printf("  %d.\t%-20.20s sold %4d  (%.1f%% of all sales)\n", shown + 1, c[i].name, c[i].sold,
               (float)c[i].sold * 100 / total);                                 /* [U1.2] typecast avoids integer division */
    if (shown == 0) printf("  No sales yet.\n");
    printf("\n*** DEAD STOCK (never sold) ***\n");
    for (int i = 0; i < count; i++)
        if (c[i].sold == 0) { printf("  -\t%-20.20s qty %d, value %.2f\n", c[i].name, c[i].quantity, c[i].quantity * c[i].price); dead++; }
    if (!dead) printf("  Every item has sold at least once.\n");
    free(c);
}
/* ===== [U2.1] 2D ARRAY: weekly sales matrix and its transpose ===== */
void weeklyMatrix(void) {
    int m[ROWS][7] = {{0}}, t[7][ROWS] = {{0}}, colTotal[7] = {0};
    int rows = MIN_OF(count, ROWS);
    if (rows == 0) { printf("No items.\n"); return; }
    for (int i = 0; i < rows; i++) for (int j = 0; j < 7; j++) t[j][i] = m[i][j] = items[i].week[j];   /* t = transpose of m */
    printf("\n*** WEEKLY SALES MATRIX (first %d items) ***\n%-18s", rows, "Item");
    for (int j = 0; j < 7; j++) printf("%5s", dayName[j]);
    printf("%7s\n", "Total");
    for (int i = 0; i < rows; i++) {
        int rowTotal = 0;
        printf("%-18.18s", items[i].name);
        for (int j = 0; j < 7; j++) { printf("%5d", m[i][j]); rowTotal += m[i][j]; colTotal[j] += m[i][j]; }
        printf("%7d\n", rowTotal);
    }
    printf("%-18s", "Day total");
    for (int j = 0; j < 7; j++) printf("%5d", colTotal[j]);
    printf("\n\nTranspose (days x item IDs):\n%-6s", "Day");
    for (int i = 0; i < rows; i++) printf("%6d", items[i].id);
    for (int j = 0; j < 7; j++) {
        printf("\n%-6s", dayName[j]);
        for (int i = 0; i < rows; i++) printf("%6d", t[j][i]);
    }
    printf("\n");
}
/* ===== [U1.4] PATTERN PRINTING ===== */
void stockChart(void) {
    printf("\n*** STOCK LEVEL CHART (each # = 5 units, max 40) ***\n");
    for (int i = 0; i < count; i++) {
        int bars = MIN_OF(items[i].quantity / 5, 40);
        if (bars == 0 && items[i].quantity > 0) bars = 1;
        printf("%-16.16s |", items[i].name);
        for (int b = 0; b < bars; b++) putchar('#');
        printf(" %d\n", items[i].quantity);
    }
}
/* ===== MEMORY INFO: heap usage and union vs struct size ===== */
void memoryInfo(void) {
    printf("\n*** MEMORY INFO ***\nItems: %d | capacity: %d (doubles with realloc) | heap in use: %lu bytes\n",
           count, capacity, (unsigned long)capacity * (unsigned long)sizeof(struct Item));
    printf("sizeof(union Measure) = %lu bytes; as a struct it would need %lu.\n-> A union shares ONE memory block.\n",
           (unsigned long)sizeof(union Measure), (unsigned long)(2 * sizeof(float) + sizeof(int)));
}
/* ===== DEMO DATA ===== */
void loadDemoData(void) {
    struct Demo { int id; const char *name, *cat; int qty; float price; int reorder, sold, expDays; unsigned char flags; char unit; float size; }
    demo[] = {
        {101, "Basmati Rice",  "Grocery",     50,  85, 10, 22, 400, F_PERISHABLE, 'W', 1},
        {102, "Wheat Flour",   "Grocery",      8, 230, 10, 15, 200, F_PERISHABLE, 'W', 5},
        {103, "Amul Milk",     "Dairy",       12,  56, 15, 40,   3, F_PERISHABLE, 'V', 1},
        {104, "Bread",         "Bakery",      20,  40, 10, 30,   2, F_PERISHABLE, 'P', 1},
        {105, "Notebook A4",   "Stationery", 100,  60, 20,  0, 9999, F_TAXABLE, 'P', 1},
        {106, "Blue Pen",      "Stationery", 200,  10, 50, 75, 9999, F_TAXABLE, 'P', 1},
        {107, "Cough Syrup",   "Medicine",     6,  95,  8,  5,  -5, F_PERISHABLE | F_TAXABLE, 'V', 0.1f},
        {108, "Cold Drink",    "Beverages",    4,  40, 10, 33,  60, F_PERISHABLE | F_TAXABLE, 'V', 0.6f}
    };
    int added = 0, n = (int)(sizeof(demo) / sizeof(demo[0]));
    for (int i = 0; i < n; i++) {
        struct Item it;
        if (findById(demo[i].id) != NULL || (count >= capacity && !growInventory(&items, &capacity))) continue;
        memset(&it, 0, sizeof(it));
        it.id = demo[i].id; it.quantity = demo[i].qty; it.price = demo[i].price; it.reorder = demo[i].reorder;
        it.sold = demo[i].sold; it.flags = demo[i].flags; it.unit = demo[i].unit;
        strncpy(it.name, demo[i].name, sizeof(it.name) - 1);
        strncpy(it.category, demo[i].cat, sizeof(it.category) - 1);
        if (it.unit == 'W') it.size.kg = demo[i].size;
        else if (it.unit == 'V') it.size.litre = demo[i].size;
        else it.size.pieces = (int)demo[i].size;
        if (demo[i].expDays != 9999) it.expiry = addDays(today(), demo[i].expDays);
        for (int k = 0; k < 7; k++) it.week[k] = it.sold / 7;                   /* spread sales across the week */
        it.week[it.id % 7] += it.sold % 7;
        items[count++] = it;
        added++;
    }
    saveData();
    logAction("DEMO data loaded");
    printf("%d demo items added.\n", added);
}
/* ===== MAIN: [U3.3] command-line arguments ===== */
int main(int argc, char *argv[]) {
    int demo = 0, choice;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) { printf("%s v%s\n", PROJECT_NAME, VERSION); return 0; }
        else if (strcmp(argv[i], "--demo") == 0) demo = 1;
        else { printf("Usage: %s [--help | --version | --demo]\n  --demo : load demo data after admin login\n", argv[0]); return strcmp(argv[i], "--help") != 0; }
    }
    initInventory(); loadData();
    printf("\n*****************************************\n*  %s - Smart Inventory, Simple C  *\n*****************************************\n", PROJECT_NAME);
    if (!login()) { printf("Too many failed attempts. Exiting.\n"); free(items); return 0; }
    printf("\nWelcome, %s!\n", user);
    if (demo && (perms & P_ADMIN)) loadDemoData();
    alerts();
    do {
        printf("\n------ %s MENU (%s) ------\n", PROJECT_NAME, user);
        printf(" 1. Add Item (admin)            9. Sort and Display\n 2. Display All Items           10. Inventory Value\n");
        printf(" 3. Search (ID / partial name)  11. Weekly Sales Matrix\n 4. Update Item (admin)         12. Stock Level Chart\n");
        printf(" 5. Delete Item (admin)         13. Fast Search by ID (binary)\n 6. Sell Items (Billing)        14. Load Demo Data (admin)\n");
        printf(" 7. Low Stock / Expiry Alerts   15. Memory Info\n 8. Top Selling / Dead Stock      0. Logout and Exit\n");
        choice = readInt("Enter choice: ");
        switch (choice) {
            case 1:  NEED(P_ADMIN) addItem(); break;
            case 2:  printTable(items, count); break;     case 3:  searchItem(); break;
            case 4:  NEED(P_ADMIN) updateItem(); break;
            case 5:  NEED(P_ADMIN) deleteItem(); break;
            case 6:  NEED(P_SELL) billing(); break;
            case 7:  alerts(); break;                     case 8:  salesReports(); break;
            case 9:  sortAndDisplay(); break;             case 11: weeklyMatrix(); break;
            case 10: printf("\nTotal inventory value: Rs. %.2f\n", totalValueRec(items, count)); break;
            case 12: stockChart(); break;                 case 13: fastSearch(); break;
            case 14: NEED(P_ADMIN) loadDemoData(); break; case 15: memoryInfo(); break;
            case 0:  logAction("LOGOUT"); printf("Data saved. Goodbye!\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    } while (choice != 0);
    free(items);                                                                /* return heap memory */
    items = NULL;
    return 0;
}