# Library Management System  
**A C++ Object-Oriented Programming Project**  
## Source Code
- main.cpp - Main C++ source code file implementing the Library Management System.
---

## Table of Contents  
- [Features](#features)  
- [Setup Instructions](#setup-instructions)  
- [File Structure](#file-structure)  
- [Default Data](#default-data)  
- [Usage Examples](#usage-examples)  
- [Fine System](#fine-system)  
- [Error Handling](#error-handling)  
- [Project Structure](#project-structure)  
- [Support](#support)  
- [License](#license)  

---

## Features  
### General Features  
- Role-based access control (Student, Faculty, Librarian)  
- CSV-based persistent data storage  
- Real-time fine calculation for overdue books  
- Automatic data saving after every operation  

### Role-Specific Capabilities  
| Role       | Borrow Limit | Borrow Period | Fine Rules                     |  
|------------|--------------|---------------|--------------------------------|  
| Student    | 3 books      | 15 days       | ₹10/day after due date         |  
| Faculty    | 5 books      | 30 days       | No fines for ≤60 days overdue  |  
| Librarian  | N/A          | N/A           | Full administrative privileges |  

---

## Setup Instructions  

### Dependencies  
- C++17 compatible compiler (GCC ≥7.0 / Clang ≥5.0)  
- Linux/macOS terminal or Windows command prompt  

### Installation  
Compile with C++17 support
g++ -std=c++17 -Wall -O2 main.cpp -o library_system

Run program
./library_system

---

## File Structure  
### CSV File Formats  
1. **Books.csv**  
   Format: `ISBN,Title,Author,Publisher,Year,Status`  
   Example:  978-0131103627,The C Programming Language,Brian W. Kernighan,Prentice Hall,1978,Available

2. **Accounts.csv**  
Format: `UserID,Role,MaxBooks,MaxDays,TotalFine,BorrowedBooks,LastFinePayment,IndividualFines,BorrowHistory`  
Example: stu001,Student,3,15,0.0,"9780131103627:1672531199",,"9780131103627:10.0","9780131103627:1672531199"

3. **Student.csv/Faculty.csv/Librarian.csv**  
Format: `Name,ID,Password`  
Example:  John Doe,stu001,stupass1


---

## Default Data  
### Preconfigured Users  
| Role       | ID      | Password    |  
|------------|---------|-------------|  
| Librarian  | lib001  | libpass     |  
| Faculty    | fac001  | facpass1    |  
| Student    | stu001  | stupass1    |  

### Sample Book Catalog  
| ISBN            | Title                                      | Status     |  
|-----------------|--------------------------------------------|------------|  
| 978-0131103627  | The C Programming Language                | Available  |  
| 978-0201633610  | Design Patterns                           | Available  |  

---

## Usage Examples  
### Student Workflow  
1. Login as stu001/stupass1

2. Borrow book (Enter ISBN) → "978-0131103627"

3. View borrowed books

4. Return book after 20 days → ₹50 fine

5. Pay fine → Clear dues

### Faculty Workflow  
1. login as Fac001/fac00

2. Borrow book (Enter ISBN) → "978-0131103627"

3. View borrowed books

4. Return book with no fine.

5. Have a book for more  60 days , no more books will be given.



### Librarian Workflow  
Login as lib001/libpass

Add new User: Name="Dr. New User", ID=fac004

Add new book: ISBN=978-1234567890, Title="New C++ Guide"

View all users



---

## Fine System  
### Rules for Fines  
- Students are charged ₹10 per day for overdue books beyond their borrowing limit of 15 days.
- Faculty members are not fined unless they exceed their extended limit of 60 days.
- Fines must be paid in full before borrowing new books.

---
### General Features:
User authentication with role-based access control (Student, Faculty, Librarian).

Persistent data storage using CSV files.

Real-time fine calculation for overdue books.

Borrowing history tracking for all users.

## Error Handling  
### Common Errors & Solutions  
1. **"Invalid credentials"**  
   - Verify CSV files contain correct user data  
   - Check for trailing spaces in input  

2. **"Book not found"**  
   - Confirm ISBN exists in Books.csv  
   - Check book status is "Available"  

3. **File I/O Errors**  
   - Ensure files have read/write permissions  
   - Verify CSV formatting matches specifications  

---

## Project Structure  
### Class Diagram  
User (Base)
├── Student
├── Faculty
└── Librarian

Book
├── ISBN
├── Title
└── Status

Account
├── BorrowHistory
├── Borrowed Books
└── FineManagement


### Key Methods  
- `User::authenticate()` - Handles login validation  
- `Librarian::addBook()` - Implements book insertion logic  
- `Student::calculateFine()` - Computes overdue charges  

---


## Support  
**Contact Developer:**  
Rohit Yadav  
  

---

## License  
This project is licensed under the IIT License - see the XYZ file for details.  

---

**Last Updated:** 9th March 2025  

---




