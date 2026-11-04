# StockSense
Console-based Inventory Management System in pure C for shops, canteens and hostel stores. Features cart billing with GST, low stock and expiry alerts, sales reports and admin/staff roles. Uses dynamic memory, structs, unions, recursion, bitwise flags and file handling. All data is saved in a binary file with a timestamped log. One 590-line C file.

<img width="756" height="907" alt="Screenshot 2026-11-04 125659" src="https://github.com/user-attachments/assets/ea677881-fc39-4aa7-bf1e-68f4e1b26863" />
<img width="737" height="846" alt="Screenshot 2026-11-04 125839" src="https://github.com/user-attachments/assets/04022e00-7da2-4a0e-80ed-07514b06990d" />
<img width="761" height="847" alt="Screenshot 2026-11-04 125905" src="https://github.com/user-attachments/assets/fad3cb3c-d294-4166-8219-c27ed548956a" />
<<img width="743" height="777" alt="Screenshot 2026-11-04 130201" src="https://github.com/user-attachments/assets/44833f6e-a540-4223-acd5-83d9cfd66404" />
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



* marks taxable items.


