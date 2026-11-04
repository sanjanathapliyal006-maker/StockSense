# StockSense
Console-based Inventory Management System in pure C for shops, canteens and hostel stores. Features cart billing with GST, low stock and expiry alerts, sales reports and admin/staff roles. Uses dynamic memory, structs, unions, recursion, bitwise flags and file handling. All data is saved in a binary file with a timestamped log. One 590-line C file.
<img width="1920" height="1080" alt="Screenshot 2026-11-03 143752" src="https://github.com/user-attachments/assets/c5fb1502-499d-4e56-a890-326003501205" />
<img width="1920" height="1080" alt="Screenshot 2026-11-03 143935" src="https://github.com/user-attachments/assets/9a0cfc74-d19f-43a6-a1e6-276d60614a72" />
<img width="1920" height="1080" alt="Screenshot 2026-11-03 144226" src="https://github.com/user-attachments/assets/09b79ae5-c575-48b9-8389-867f9d4a06e0" />
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

Quick start: run ./stocksense --demo, log in as admin, and try the menu options.

Menu
for example :  1. Add Item (admin)            9. Sort and Display
 2. Display All Items           10. Inventory Value
 3. Search (ID / partial name)  11. Weekly Sales Matrix
 4. Update Item (admin)         12. Stock Level Chart
 5. Delete Item (admin)         13. Fast Search by ID (binary)
 6. Sell Items (Billing)        14. Load Demo Data (admin)
 7. Low Stock / Expiry Alerts   15. Memory Info
 8. Top Selling / Dead Stock     0. Logout and Exit
Sample bill
=============== StockSense ===============
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

* marks taxable items.


