# StockSense
Console-based Inventory Management System in pure C for shops, canteens and hostel stores. Features cart billing with GST, low stock and expiry alerts, sales reports and admin/staff roles. Uses dynamic memory, structs, unions, recursion, bitwise flags and file handling. All data is saved in a binary file with a timestamped log. One 590-line C file.
Secure login with role-based access. The menu shows all 15 options, and admin-only actions are marked.
<img width="756" height="907" alt="Screenshot 2026-11-04 125659" src="https://github.com/user-attachments/assets/ea677881-fc39-4aa7-bf1e-68f4e1b26863" />
Item entry with category, price, reorder level, unit type, perishable and GST flags, and expiry date.
<img width="737" height="846" alt="Screenshot 2026-11-04 125839" src="https://github.com/user-attachments/assets/04022e00-7da2-4a0e-80ed-07514b06990d" />
Table with size, flags (P, T), expiry date and live status: OK, LOW STOCK or EXPIRED.
<img width="761" height="847" alt="Screenshot 2026-11-04 125905" src="https://github.com/user-attachments/assets/fad3cb3c-d294-4166-8219-c27ed548956a" />
Capacity doubles with realloc, and a union uses 4 bytes where a struct would need 12.
<<img width="743" height="777" alt="Screenshot 2026-11-04 130201" src="https://github.com/user-attachments/assets/44833f6e-a540-4223-acd5-83d9cfd66404" />
Items below reorder level, items expiring within 7 days, and expired items.
<img width="743" height="777" alt="Screenshot 2026-11-04 130201" src="https://github.com/user-attachments/assets/0ec0cd42-2447-4166-8b77-cac63caffe50" />

Features
Area	What it does
Inventory (CRUD)	Add, display, update and delete items, with input validation and confirmation prompts
Smart search	Search by ID, or by part of a name (case-insensitive: ric finds Basmati Rice)
Cart billing	Multiple items per bill, GST on taxable items only, auto bill number, bill saved as bill_<no>.txt
Alerts	Low stock alert and expiry alert (expired items cannot be sold)
Reports	Top selling items with sales share, dead stock, total inventory value
Sorting	Sort by name, price or quantity (bubble sort)
Fast search	Binary search by ID (recursive)
Weekly sales matrix	Items x weekdays table with totals and its transpose
Stock chart	Bar chart of stock levels using #
Role-based access	admin has full access; staff can view, sell and see reports
Persistence	Data saved in a binary file, activity saved in a timestamped text log
Demo mode	Loads sample items with realistic expiry dates in one step
Getting Started
Requirements
Any C compiler (GCC recommended)
Compile
bash
gcc stocksense.c -o stocksense
Run
bash
./stocksense            # Linux / macOS
stocksense.exe          # Windows
Command-line options
bash
./stocksense --help      # show usage
./stocksense --version   # show version
./stocksense --demo      # load demo data after admin login
Login credentials
Role	Username	Password
Admin	admin	admin123
Staff	staff	staff123
 1. Add Item (admin)            9. Sort and Display
 2. Display All Items           10. Inventory Value
 3. Search (ID / partial name)  11. Weekly Sales Matrix
 4. Update Item (admin)         12. Stock Level Chart
 5. Delete Item (admin)         13. Fast Search by ID (binary)
 6. Sell Items (Billing)        14. Load Demo Data (admin)
 7. Low Stock / Expiry Alerts   15. Memory Info
 8. Top Selling / Dead Stock     0. Logout and Exit
Sample Input     =============== StockSense ===============
Bill No : 1001
Cashier : admin
------------------------------------------------
No   Item                 Qty   Rate     Amount
1    Basmati Rice         2     85.00    170.00
2    Notebook A4          2     60.00    120.00 *
------------------------------------------------
Subtotal          :     290.00
GST (5% on *)     :       6.00
GRAND TOTAL       :     296.00
================================================
        Thank you! Visit again.
        Where to look in the code
Concept	Example in stocksense.c
Dynamic array that grows	growInventory() doubles memory with realloc through a struct Item **
Bitwise operators	Item flags (F_PERISHABLE, F_TAXABLE) and role permissions (P_ADMIN, P_SELL) are bit masks
Union	union Measure stores weight, volume or piece count in the same 4 bytes
Recursion	totalValueRec() and binarySearchRec()
2D array	weeklyMatrix() builds a matrix and its transpose
Pointer arithmetic	findById() and printTable() walk the array with pointers
goto	Error handling in loadData() and cleanup in billing()
Macros	CALC_GST(x), MIN_OF(a, b), NEED(p), #ifndef, #ifdef DEBUG
Command-line arguments	main(int argc, char *argv[]) handles --help, --version, --demo

Files Created at Runtime
File	Type	Purpose
inventory.dat	Binary	Saved inventory and last bill number
log.txt	Text	Timestamped record of logins, sales, additions, updates and deletions
bill_<no>.txt	Text	One file per bill
Debug mode 

Limitations and Roadmap

Current limitations

Fixed demo credentials (a real system would use hashed passwords and a user database)
Data stored in a flat binary file
Console interface only.





