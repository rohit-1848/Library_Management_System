#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
using namespace std;

class User;
class Book;
class Account;
class Student;
class Librarian;
class Faculty;
//global temporary variable
map<string,Student> gStudent;        //userID --> Student class
map<string,Faculty> gFaculty;        //userID --> Faculty class
map<string,Librarian> gLibrarian;    //userID --> Librarian class
map<string, Book> gBook;             //isbn number--> Book calss
map<string, Account> gAccount;       // userID --> Account class

//imp Functions
// Returns current date/time in seconds since epoch (January 1, 1970)
int getCurrentDateInSeconds() {
    return static_cast<int>(time(nullptr));
}

//================================CLASSESS=========================
class Library; // Forward declaration of Library

//book Class
class Book {
private:
    string title;
    string author;
    string publisher;
    int year;
    string isbn;
    string status; // "available", "borrowed", or "reserved"

public:
    // Constructor
    Book(string t = "", string a = "", string p = "", int y = 0, string i = "", string s = "available")
        : title(t), author(a), publisher(p), year(y), isbn(i), status(s) {}

    // Display book details
    void displayBook() const {
        cout << "Title: " << title << ", Author: " << author
             << ", Publisher: " << publisher << ", Year: " << year
             << ", ISBN: " << isbn << ", Status: " << status << endl;
    }

    // Getter methods
    string getISBN() const { return isbn; }
    string getStatus() const { return status; }
    void setStatus(const string& newStatus) { status = newStatus; }

    // Serialize book data into CSV format for file storage
    void saveToFile(ofstream &outfile) const {
        outfile << isbn << "," << title << "," << author << ","
                << publisher << "," << year << "," << status << "\n";
    }

    //Load book details from CSV file
    void loadFromFile(ifstream &infile) {
        string line;
        if (getline(infile, line)) {
            stringstream ss(line);
            getline(ss, isbn, ',');
            getline(ss, title, ',');
            getline(ss, author, ',');
            getline(ss, publisher, ',');
            
            string yearStr;
            getline(ss, yearStr, ',');
            year = stoi(yearStr);

            getline(ss, status);
        }
    }

    // Reserve the book if available
    void reserve() {
        if (status == "available") {
            status = "reserved";
            cout << "\"" << title << "\" has been reserved successfully.\n";
        } else {
            cout << "\"" << title << "\" is currently " << status 
                 << ". Reservation not possible.\n";
        }
    }

    // Getter functions
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    int getYear() const { return year; }
};

//Use Account class
class Account {
private:
    friend class User;  // Allow User class access to private members

    string userID;
    string role;  //sSudent /Facualty / Librarian
    int maxBooks;
    int maxDays;

    map<string, int> borrowedBooks;            // ISBN -> due date (epoch time) OR (in seconds since epoch)
    map<string, int> lastFinePayment;          // ISBN -> last fine payment date (epoch time)
    mutable map<string, double> individualFines; // ISBN -> fine amount for perticular book
    mutable double totalFine;

    vector<pair<string, int>> borrowHistory;   // ISBN -> return date (epoch time)

public:
    // Constructor
    Account(string id = "", string role ="Student")
        : userID(id), role(role), 
          maxBooks((role=="Faculty") ? 5 : 3), maxDays((role=="Faculty") ? 30 : 15),totalFine(0) {}

    // Getter methods
    string getUserID() const { return userID; }
    double getTotalFine() const { return totalFine; }
    string getRole() const { return role; }
    int getMaxBooks() const { return maxBooks; }
    int getMaxDays() const { return maxDays; }
    
    const map<string, int>& getBorrowedBooks() const { return borrowedBooks; }
    const vector<pair<string, int>>& getBorrowHistory() const { return borrowHistory; }
    
    double getFineForBook(const string& isbn) const {
        auto it = individualFines.find(isbn);
        return (it != individualFines.end()) ? it->second : 0;
    }
    //adder methods
    void addHistory(const string& isbn, int currentDate){
        borrowHistory.push_back({isbn,currentDate});
    }
    void addBorrowedBook(const string& isbn, int dueDate){
        borrowedBooks[isbn]=dueDate;
    }
    void deleteBorrowedBooks(const string& isbn){
        borrowedBooks.erase(isbn);
    }
    // show methods
    void showBorrowHistory(){
        cout<<"Books borrowed by user:"<<endl;
        for (const auto& it:borrowHistory){
            cout<<"Book ISBN:"<<it.first<< "returned on Date:";
            displayDate(it.second);
            cout<<"."<<endl;
        }
    }
    void showCurrentBorrowedBooks(){
        cout<<"You have currently borrowed these books:"<<endl;
        for(const auto& it: borrowedBooks){
            cout<<"Book ISBN:"<<it.first<< "is borrowed and should be returned before ";
            displayDate(it.second);
            cout<<"(due date)."<<endl;
        }
    }
    // Display account details clearly
    void displayAccountDetails(int currentDate) const {
        updateAllFines(currentDate);

        cout << "\n--- Account Details ---\n";
        cout << "User ID: " << userID 
             << "\nType: " << role
             << "\nMax Books Allowed: " << maxBooks
             << "\nBorrowing Period: " << maxDays <<" days"
             << "\nTotal Fine: Rs."<<totalFine<<"\n";

        cout<<"\nCurrently Borrowed Books:\n";
        
        if(borrowedBooks.empty())
            cout<<"None\n";
        
        for(auto &[isbn,due]:borrowedBooks){
          cout<<"ISBN:"<<isbn<<" | Due Date:";
          displayDate(due);
          double fine=getFineForBook(isbn);
          if(fine>0)
              cout<<" | Overdue Fine:Rs."<<fine<<"\n";
          else
              cout<<" | Status:On Time\n";
         }

         cout<<"\nBorrowing History:\n";
         if(borrowHistory.empty())
             cout<<"No history available.\n";

         for(auto &[isbn,date]:borrowHistory){
           cout<<"ISBN:"<<isbn<<" | Returned on:";
           displayDate(date);
           cout<<endl;}
     }
    // Pay fine for a specific book
    bool payFineForBook(const string& isbn, double amount, int currentDate) {
        updateAllFines(currentDate);

        if (role=="Faculty" || role=="Librarian") {
            cout << "Faculty or Librarian members have no fines.\n";
            return true;
        }

        auto fineIt = individualFines.find(isbn);
        if (fineIt == individualFines.end() || fineIt->second != amount) {
            cout << "Incorrect fine amount. Please pay exact fine: Rs." << fineIt->second << endl;
            return false;
        }

        totalFine -= fineIt->second;
        individualFines.erase(fineIt);

        borrowedBooks[isbn] = currentDate + maxDays * 86400;
        lastFinePayment[isbn] = currentDate;

        cout << "Payment successful! Book reissued until ";
        displayDate(borrowedBooks[isbn]);
        cout << endl;

        return true;
    }

    // Pay total fines
    bool payTotalFine(double amount, int currentDate) {
        updateAllFines(currentDate);

        if (role=="Faculty" || totalFine == 0) {
            cout << "No fines to pay.\n";
            return true;
        }

        if (amount != totalFine) {
            cout << "Incorrect payment. Exact total fine is Rs." << totalFine << endl;
            return false;
        }

        totalFine = 0;
        individualFines.clear();

        for (auto& [isbn, dueDate] : borrowedBooks) {
            borrowedBooks[isbn] = currentDate + maxDays * 86400;
            lastFinePayment[isbn] = currentDate;
            cout << "Book " << isbn << " reissued until ";
            displayDate(borrowedBooks[isbn]);
            cout << endl;
        }

        cout << "All fines cleared successfully.\n";
        return true;
    }

    // Return a borrowed book
    bool returnBorrowedBook(const string& isbn, int currentDate) {  //current  date is in system seconds formate
        auto it = borrowedBooks.find(isbn);
        if(role=="Student"){   
            updateAllFines(currentDate);

            if (it == borrowedBooks.end()) {
                cout << "Book not found in your borrowed list.\n";
                return false;
            }

            int overdueSeconds = currentDate - it->second;
            
            if (overdueSeconds > 0 && !(role=="Faculty")) {
                double fineDue = (overdueSeconds / 86400) * 10.0;
                cout << "Book overdue by " << overdueSeconds / 86400 
                    << " days. Fine: Rs." << fineDue << "\nPay now? (1-Yes/0-No): ";
                
                int choice; cin >> choice;

                if (choice == 1 && !payFineForBook(isbn, fineDue, currentDate)) {
                    cout << "Payment failed. Return aborted.\n";
                    return false;
                } else if (choice == 0) {
                    cout << "Return aborted. Fine must be cleared first.\n";
                    return false;
                }
            }
        }
        borrowHistory.emplace_back(isbn, currentDate); //current date== retrun date
        borrowedBooks.erase(it);
        gBook[isbn].setStatus("available");
        cout << "Book returned successfully.\n";
        
        return true;
    }


    void displayDate(int epochTime)const{  //displays date in Y/M/D formate ,the epochTime parameter represents the date in seconds.
        time_t t=epochTime;tm*tmPtr=localtime(&t);
        char buf[20];strftime(buf,sizeof(buf),"%Y-%m-%d",tmPtr);
        cout<<buf;}
    
    //=====================LOAD & SAVE========================
    void saveToFile(ofstream &outfile) const {
        // Write basic attributes: userID, role, maxBooks, maxDays, totalFine
        outfile << userID << "," << role << "," << maxBooks << "," << maxDays << "," << totalFine << ",";
        
        // Write borrowedBooks map (ISBN:dueDate;...)
        for (const auto& pair : borrowedBooks) {
            outfile << pair.first << ":" << pair.second << ";";
        }
        outfile << ",";
    
        // Write lastFinePayment map (ISBN:lastPaymentDate;...)
        for (const auto& pair : lastFinePayment) {
            outfile << pair.first << ":" << pair.second << ";";
        }
        outfile << ",";
    
        // Write individualFines map (ISBN:fineAmount;...)
        for (const auto& pair : individualFines) {
            outfile << pair.first << ":" << pair.second << ";";
        }
        outfile << ",";
        
        // Write borrowHistory vector (ISBN:returnDate;...)
        for (const auto& entry : borrowHistory) {
            outfile << entry.first << ":" << entry.second << ";";
        }
        
        outfile << "\n"; // End of this account's data
    }
        
    // Load book details from CSV file
    void loadFromFile(ifstream &infile) {
        string line;
        if (!getline(infile, line)) return;
    
        stringstream ss(line);
        
        // Temporary strings to hold data
        string maxBooksStr, maxDaysStr, totalFineStr;
        string borrowedBooksStr, lastFinePaymentStr, individualFinesStr, borrowHistoryStr;
    
        // Read basic attributes
        getline(ss, userID, ',');
        getline(ss, role, ',');
        getline(ss, maxBooksStr, ',');
        getline(ss, maxDaysStr, ',');
        getline(ss, totalFineStr, ',');
    
        maxBooks = stoi(maxBooksStr);
        maxDays = stoi(maxDaysStr);
        totalFine = stod(totalFineStr);
    
        // Read borrowedBooks map
        borrowedBooks.clear();
        getline(ss, borrowedBooksStr, ',');
        stringstream bbStream(borrowedBooksStr);
        string bbEntry;
        while (getline(bbStream, bbEntry, ';')) {
            if (!bbEntry.empty()) {
                size_t pos = bbEntry.find(':');
                if (pos != string::npos) {
                    string isbn = bbEntry.substr(0, pos);
                    int dueDate = stoi(bbEntry.substr(pos + 1));
                    borrowedBooks[isbn] = dueDate;
                }
            }
        }
    
        // Read lastFinePayment map
        lastFinePayment.clear();
        getline(ss, lastFinePaymentStr, ',');
        stringstream lfpStream(lastFinePaymentStr);
        string lfpEntry;
        while (getline(lfpStream, lfpEntry, ';')) {
            if (!lfpEntry.empty()) {
                size_t pos = lfpEntry.find(':');
                if (pos != string::npos) {
                    string isbn = lfpEntry.substr(0, pos);
                    int paymentDate = stoi(lfpEntry.substr(pos + 1));
                    lastFinePayment[isbn] = paymentDate;
                }
            }
        }
    
        // Read individualFines map
        individualFines.clear();
        getline(ss, individualFinesStr, ',');
        stringstream finesStream(individualFinesStr);
        string fineEntry;
        while (getline(finesStream, fineEntry, ';')) {
            if (!fineEntry.empty()) {
                size_t pos = fineEntry.find(':');
                if (pos != string::npos) {
                    string isbn = fineEntry.substr(0, pos);
                    double fineAmount = stod(fineEntry.substr(pos + 1));
                    individualFines[isbn] = fineAmount;
                }
            }
        }
    
        // Read borrowHistory vector
        borrowHistory.clear();
        getline(ss, borrowHistoryStr);
        
        stringstream historyStream(borrowHistoryStr);
        
        string historyEntry;
        
         while(getline(historyStream ,historyEntry,';')){
             if(!historyEntry.empty()){
                 size_t pos=historyEntry.find(':');
                 if(pos!=string::npos){
                     string isbn=historyEntry.substr(0,pos);
                     int returnDate=stoi(historyEntry.substr(pos+1));
                     borrowHistory.push_back({isbn ,returnDate});
                 }
             }
         }
    }
    // void saveToFile(ofstream &outfile) const {
    //     // Write basic attributes: userID, role, maxBooks, maxDays, totalFine
    //     outfile << userID << "," << role << "," << maxBooks << "," << maxDays << "," << totalFine << ",";
        
    //     // Write borrowedBooks map (ISBN:dueDate;...)
    //     for (const auto& pair : borrowedBooks) {
    //         outfile << pair.first << ":" << pair.second << ";";
    //     }
    //     outfile << ",";
        
    //     // Write lastFinePayment map (ISBN:lastPaymentDate;...)
    //     for (const auto& pair : lastFinePayment) {
    //         outfile << pair.first << ":" << pair.second << ";";
    //     }
    //     outfile << ",";
        
    //     // Write individualFines map (ISBN:fineAmount;...)
    //     for (const auto& pair : individualFines) {
    //         outfile << pair.first << ":" << pair.second << ";";
    //     }
    //     outfile << ",";
        
    //     // Write borrowHistory vector (ISBN:returnDate;...)
    //     for (const auto& entry : borrowHistory) {
    //         outfile << entry.first << ":" << entry.second << ";";
    //     }
        
    //     outfile << "\n"; // End of this account's data
    // }
    // void loadFromFile(ifstream &infile) {
    //     string line;
    //     if (!getline(infile, line)) return;
    
    //     stringstream ss(line);
    
    //     // Temporary strings to hold data
    //     string maxBooksStr, maxDaysStr, totalFineStr;
    //     string borrowedBooksStr, lastFinePaymentStr, individualFinesStr, borrowHistoryStr;
    
    //     // Read basic attributes
    //     getline(ss, userID, ',');
    //     getline(ss, role, ',');
    //     getline(ss, maxBooksStr, ',');
    //     getline(ss, maxDaysStr, ',');
    //     getline(ss, totalFineStr, ',');
    
    //     // Convert basic attributes to correct types
    //     try {
    //         maxBooks = stoi(maxBooksStr);
    //         maxDays = stoi(maxDaysStr);
    //         totalFine = stod(totalFineStr);
    //     } catch (const invalid_argument& e) {
    //         cerr << "Error parsing account attributes for UserID: " << userID << endl;
    //         return;
    //     }
    
    //     // Read borrowedBooks map
    //     borrowedBooks.clear();
    //     getline(ss, borrowedBooksStr, ',');
    //     stringstream bbStream(borrowedBooksStr);
    //     string bbEntry;
    //     while (getline(bbStream, bbEntry, ';')) {
    //         if (!bbEntry.empty()) {
    //             size_t pos = bbEntry.find(':');
    //             if (pos != string::npos) {
    //                 string isbn = bbEntry.substr(0, pos);
    //                 int dueDate = stoi(bbEntry.substr(pos + 1));
    //                 borrowedBooks[isbn] = dueDate;
    //             }
    //         }
    //     }
    
    //     // Read lastFinePayment map
    //     lastFinePayment.clear();
    //     getline(ss, lastFinePaymentStr, ',');
    //     stringstream lfpStream(lastFinePaymentStr);
    //     string lfpEntry;
    //     while (getline(lfpStream, lfpEntry, ';')) {
    //         if (!lfpEntry.empty()) {
    //             size_t pos = lfpEntry.find(':');
    //             if (pos != string::npos) {
    //                 string isbn = lfpEntry.substr(0, pos);
    //                 int lastPaymentDate = stoi(lfpEntry.substr(pos + 1));
    //                 lastFinePayment[isbn] = lastPaymentDate;
    //             }
    //         }
    //     }
    
    //     // Read individualFines map
    //     individualFines.clear();
    //     getline(ss, individualFinesStr, ',');
    //     stringstream finesStream(individualFinesStr);
    //     string fineEntry;
    //     while (getline(finesStream, fineEntry, ';')) {
    //         if (!fineEntry.empty()) {
    //             size_t pos = fineEntry.find(':');
    //             if (pos != string::npos) {
    //                 string isbn = fineEntry.substr(0, pos);
    //                 double fineAmount = stod(fineEntry.substr(pos + 1));
    //                 individualFines[isbn] = fineAmount;
    //             }
    //         }
    //     }
    
    //     // Read borrowHistory vector
    //     borrowHistory.clear();
    //     getline(ss, borrowHistoryStr);
    //     stringstream historyStream(borrowHistoryStr);
    //     string historyEntry;
    //     while (getline(historyStream, historyEntry, ';')) {
    //         if (!historyEntry.empty()) {
    //             size_t pos = historyEntry.find(':');
    //             if (pos != string::npos) {
    //                 string isbn = historyEntry.substr(0, pos);
    //                 int returnDate = stoi(historyEntry.substr(pos + 1));
    //                 borrowHistory.emplace_back(isbn, returnDate);
    //             }
    //         }
    //     }
    // }
        

private:
    void updateAllFines(int currentDate)const{  //may get error
        if(role=="Faculty")return;

        totalFine=0;

        for(auto &[isbn,due]:borrowedBooks){
            int overdue=currentDate-due;
            if(overdue>0){
                double fineAmt=(overdue/86400)*10.0;
                individualFines[isbn]=fineAmt;
                totalFine+=fineAmt;}
            else individualFines.erase(isbn);}
    }


};

// User Base Class
class User {
protected:
    string name;
    string userID;
    string password;
public:

    Account* account;
    User(string n="", string id="", string pwd="User@123",string role="Student"): name(n), userID(id), password(pwd){   //may get error at account()
        // Create a new account only if one doesn't exist
        if (gAccount.find(userID) == gAccount.end()) {
            gAccount[userID] = Account(userID, role);
            account=&gAccount[userID]; //saving the reference to the user account
        } else {
            // Update existing account
            account=&gAccount[userID];
            gAccount[userID].role = role;
            gAccount[userID].maxBooks = (role=="Faculty") ? 5 : 3;
            gAccount[userID].maxDays = (role=="Faculty") ? 30 : 15;
        }
        gAccount[userID].userID = userID;
    }
    
    virtual void borrowBook(const string& isbn, int currentDate)=0;
    virtual void returnBook(const string& isbn, int currentDate)=0;
    virtual void displayInfo() const {
        cout << "User: " << name << ", ID: " << userID << endl;
    }
    string getName(){
        return name;
    }
    string getPassword(){
        return password;
    }
    string getID(){
        return userID;
    }
    // Declaring Librarian as a friend class
    friend class Librarian;
};

// Student Class
class Student : public User {
private:
    static const int maxBooks = 3;

public:
    Student(string n ="", string id="", string pwd="") : User(n, id, pwd, "Student"){} 

    void borrowBook(const string& isbn, int currentDate) override{ // curretndate is in system seconds formate
        //check if book is available of not in library
        auto it = gBook.find(isbn);
        if (it == gBook.end() || it->second.getStatus()=="Avialable") {
            cout << "Book not available in Library.\n";
            return ;
        }
        //debugging
        cout<<account->getBorrowedBooks().size()<<","<<gBook[isbn].getStatus()<<","<<account->getTotalFine()<<endl;
        if(account->getBorrowedBooks().size()<maxBooks && gBook[isbn].getStatus()=="available" && account->getTotalFine() ==0){
            account->addBorrowedBook(isbn,currentDate+15*60*60*24);
            gBook[isbn].setStatus("borrowed");
            cout<<name<<" borrowed "<<gBook[isbn].getISBN()<<endl;
            cout <<endl<< "Book ISBN:"<<isbn<<" is borrowed succesfully on date:";
            account->displayDate(currentDate);
            cout<<"."<<endl;
            cout <<"Book should be returned before Date:" ;
            account->displayDate((currentDate+15*60*60*24));
            cout<<"."<<endl;
        }
        else if(account->getBorrowedBooks().size()>=maxBooks){
            cout<<" You can't borrow more books as your borrowing limit(max 3 books) is reached.\n";
            cout<<"Please return one of the book to procced"<<endl;
        }
        else if(account->getTotalFine() !=0){
            cout<<"You have unpaid fines , Please pay all the fines first."<<endl;
        }
        else{
            cout<<"Book is not available for borrowing. \n ";
        }
    }

    void returnBook(const string& isbn, int currentDate) override{
        account->returnBorrowedBook(isbn,currentDate);
    }


};

// Faculty Class
class Faculty : public User {
private:
   static const int maxBooks = 5;

public:
    Faculty(string n ="", string id="", string pwd="") : User(n, id, pwd, "Faculty"){} 
    void borrowBook(const string& isbn, int currentDate) override{ // curretndate is in system seconds formate
        //check if book is available of not in library
        auto it = gBook.find(isbn);
        if (it == gBook.end() || it->second.getStatus()=="Avialable") {
            cout << "Book not available in Library.\n";
            return ;
        }
        // check for if there is any book with 60 plus overdue days
        bool check4ExtendedOverdueBook=false;
        for (auto& [isbn, dueDate] : account->getBorrowedBooks()) {
            int diff= (currentDate-dueDate)/(60*60*24);
            if(diff>60){
                check4ExtendedOverdueBook=true;
            }
        }
        if(account->getBorrowedBooks().size()<maxBooks && gBook[isbn].getStatus()=="available" && check4ExtendedOverdueBook==false){
            account->addBorrowedBook(isbn,currentDate+15*60*60*24);
            gBook[isbn].setStatus("borrowed");
            cout<<name<<" borrowed "<<gBook[isbn].getISBN()<<endl;
            cout <<endl<< "Book ISBN:"<<isbn<<" is borrowed succesfully on date:";
            account->displayDate(currentDate);
            cout<<"."<<endl;
            cout <<"Book should be returned before Date:" ;
            account->displayDate((currentDate+30*60*60*24));
            cout<<"."<<endl;
            return;
        }
        else if(account->getBorrowedBooks().size()>=maxBooks){
            cout<<" You can't borrow more books as your borrowing limit(max 5 books) is reached.\n";
            cout<<"Please return one of the book to procced"<<endl;
            return;
        }
        else if(check4ExtendedOverdueBook==true){
            cout<<"You have an overdue book for more then 60 days."<<endl;
            cout<<"Please return the overdued books to procced"<<endl;
            return;
        }
        else{
            cout<<"Book is not available for borrowing. \n ";
        }
    }

    void returnBook(const string& isbn, int currentDate) override{
        account->returnBorrowedBook(isbn,currentDate);
    }
};

// Librarian Class
class Librarian : public User {
public:
    Librarian(string n ="", string id="", string pwd="") : User(n, id, pwd, "Librarian"){} 
    void borrowBook(const string& isbn, int currentDate)override{cout<<"Librarians can't borrow books.\n";}
    void returnBook(const string& isbn, int currentDate)override{cout<<"Librarians can't return books.\n";}
    
    void addUser(string name, string id,string pwd, string role ){
        if(role=="Student"){
            if (gStudent.find(id) != gStudent.end()) {
                cout << "Error: User ID already exists!\n";
                return;
            }
            Student* temp=new Student(name,id,pwd);
            gStudent[id]= *temp;
            cout<<"User is sucessfully enrolled"<<endl;
        }
        else if(role=="Faculty"){
            if (gFaculty.find(id) != gFaculty.end()) {
                cout << "Error: User ID already exists!\n";
                return;
            }
            Faculty* temp=new Faculty(name,id,pwd);
            gFaculty[id]= *temp;
            cout<<"User is sucessfully enrolled"<<endl;
        }
        else if(role=="Librarian"){
            if (gLibrarian.find(id) != gLibrarian.end()) {
                cout << "Error: User ID already exists!\n";
                return;
            }
            Librarian* temp=new Librarian(name,id,pwd);
            gLibrarian[id]= *temp;
            cout<<"User is sucessfully enrolled"<<endl;
        }
        else{
            cout<<"Invalid user type , Role does not exist"<<endl;
        }
    }

    void removeUser(string id, string role){
        if(role=="Student"){
            if (gStudent.find(id) != gStudent.end()) {
                gStudent.erase(id);
                cout<<"User succesfully removed"<<endl;
                return;
            }
            cout << "Error: User ID doesn't exists!\n";
        }
        else if(role=="Faculty"){
            if (gFaculty.find(id) != gFaculty.end()) {
                gFaculty.erase(id);
                cout<<"User succesfully removed"<<endl;
                return;
            }
            cout << "Error: User ID doesn't exists!\n";
        }
        else if(role=="Librarian"){
            if (gLibrarian.find(id) != gLibrarian.end()) {
                gLibrarian.erase(id);
                cout<<"User succesfully removed"<<endl;
                return;
            }
            cout << "Error: User ID doesn't exists!\n";
        }
        else{
            cout<<"Invalid user type , Role does not exist"<<endl;
        }
    }

    void addBook(string title,string author,string publisher,int year,
                 string isbn,string status){
        if(gBook.find(isbn) != gBook.end()){
            cout<<"Book already exist."<<endl;
            cout<<"Book is Updated "<<endl;
            gBook[isbn]=Book(title,author,publisher,year,isbn,status);
            return;
        }
        Book* temp=new Book(title,author,publisher,year,isbn,status);
        gBook[isbn]= *temp;
        cout<<"Book title:"<<title<<"succesfully added to Library"<<endl;
    }

    void removeBook(string isbn){
        if(gBook.find(isbn) != gBook.end()){
            gBook.erase(isbn);
            cout<<"Book is succesfully removed from library."<<endl;
            return;
        }
        else{
            cout<<"Error:Book isbn:"<<isbn<<" doesn't exist in Library."<<endl;
        }
    }
};

// Library Class (Manages Books and Users)
class Library {
private:
    /*map<string, Student> gStudent;      // userID --> Student class
    map<string, Faculty> gFaculty;    // userID --> Faculty class
    map<string, Librarian> gLibrarian; // userID --> Librarian class
    map<string, Book> gBook;           // ISBN --> Book class
    map<string, Account> gAccount;     // userID --> Account class */

public:
    // Getter methods for maps (to allow controlled access)
    // map<string, Student>& getStudents() { return gStudent; }
    // map<string, Faculty>& getFaculties() { return gFaculty; }
    // map<string, Librarian>& getLibrarians() { return gLibrarian; }
    // map<string, Book>& getBooks() { return gBook; }
    // map<string, Account>& getAccounts() { return gAccount; }
    void dispalyAllBooks() const{
        cout<<"Books that provided by Library:"<<endl;
        for(const auto& it: gBook ){
            it.second.displayBook();
        }
    }
    void viewAllUsers() const;
    //Load data functions
    void loadStudentsFromFile(const string& filename);
    void loadFacultiesFromFile(const string& filename);
    void loadLibrariansFromFile(const string& filename);
    void loadBooksFromFile(const string& filename);
    void loadAccountsFromFile(const string& filename);

    // Save data functions
    void saveStudentsToFile(const string& filename);
    void saveFacultiesToFile(const string& filename);
    void saveLibrariansToFile(const string& filename);
    void saveBooksToFile(const string& filename);
    void saveAccountsToFile(const string& filename);

    void loadLibrary(){
        cout << "Loading Library...\n";
        try {
            loadStudentsFromFile("Student.csv");
            loadFacultiesFromFile("Faculty.csv");
            loadLibrariansFromFile("Librarian.csv");
            loadBooksFromFile("Books.csv");
            loadAccountsFromFile("Account.csv");
            cout << "Library loaded successfully.\n";
        } catch (const exception& e) {
            cerr << "Error loading Library: " << e.what() << "\n";
        }
    }
    void saveLibrary(){
        cout << "Saving Library...\n";
        try {
            saveStudentsToFile("Student.csv");
            saveFacultiesToFile("Faculty.csv");
            saveLibrariansToFile("Librarian.csv");
            saveBooksToFile("Books.csv");
            saveAccountsToFile("Account.csv");
            cout << "Library saved successfully.\n";
        } catch (const exception& e) {
            cerr << "Error loading Library: " << e.what() << "\n";
        }
    }
    // Friend classes to allow access to private members
    friend class Account;
    friend class User;
};
void Library::viewAllUsers() const{
    cout << "=== List of All Users ===\n";

    // Display all students
    if (!gStudent.empty()) {
        cout << "\n--- Students ---\n";
        for ( auto& [id, student] : gStudent) {
            cout << "ID: " << id << ", Name: " << student.getName() << ", Role: Student\n";
        }
    } else {
        cout << "\nNo students found.\n";
    }

    // Display all faculty members
    if (!gFaculty.empty()) {
        cout << "\n--- Faculty Members ---\n";
        for ( auto& [id, faculty] : gFaculty) {
            cout << "ID: " << id << ", Name: " << faculty.getName() << ", Role: Faculty\n";
        }
    } else {
        cout << "\nNo faculty members found.\n";
    }

    // Display all librarians
    if (!gLibrarian.empty()) {
        cout << "\n--- Librarians ---\n";
        for ( auto& [id, librarian] : gLibrarian) {
            cout << "ID: " << id << ", Name: " << librarian.getName() << ", Role: Librarian\n";
        }
    } else {
        cout << "\nNo librarians found.\n";
    }

    cout << "\n=== End of User List ===\n";
}

//Library Functions
void Library::loadStudentsFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string id, name, password;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, password);
        if (name.empty() || id.empty() || password.empty()) {
            cerr << "[ERROR] Skipped invalid student record.\n";
            continue; // skip this row
        }
        gStudent[id] = Student(name, id, password); // Add student to the map
    }
    cout<<"Students loaded from file Student.csv Successfully."<<endl;
    infile.close();
}
void Library::saveStudentsToFile(const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    for ( auto& pair : gStudent) {
        outfile << pair.first << "," // userID
                << pair.second.getName() << "," // name
                << pair.second.getPassword() << "\n"; // password
    }
    cout<<"Students saved to file Student.csv Successfully."<<endl;
    outfile.close();
}
void Library::loadFacultiesFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string id, name, password;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, password);

        if (name.empty() || id.empty() || password.empty()) {
            cerr << "[ERROR] Skipped invalid faculty record.\n";
            continue; // skip this row
        }
        gFaculty[id] = Faculty(name, id, password); // Add faculty to the map
    }
    cout<<"Faculties loaded from file Faculty.csv Successfully."<<endl;
    infile.close();
}
void Library::saveFacultiesToFile(const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    for ( auto& pair : gFaculty) {
        outfile << pair.first << "," // userID
                << pair.second.getName() << "," // name
                << pair.second.getPassword() << "\n"; // password
    }
    cout<<"Faculties saved to file Faculty.csv Successfully."<<endl;
    outfile.close();
}
void Library::loadLibrariansFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string id, name, password;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, password);
        if (name.empty() || id.empty() || password.empty()) {
            cerr << "[ERROR] Skipped invalid Librarian record.\n";
            continue; // skip this row
        }
        gLibrarian[id] = Librarian(name, id, password); // Add librarian to the map
    }
    cout<<"Lbrarians loaded from file Librarian.csv Successfully."<<endl;

    infile.close();
}
void Library::saveLibrariansToFile(const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    for ( auto& pair : gLibrarian) {
        outfile << pair.first << "," // userID
                << pair.second.getName() << "," // name
                << pair.second.getPassword() << "\n"; // password
    }
    cout<<"Librarians saved to file Librarian.csv Successfully."<<endl;
    outfile.close();
}
// void Library::loadBooksFromFile(const string& filename) {
//     ifstream infile(filename);
//     if (!infile.is_open()) {
//         cout << "Error: Could not open file " << filename << endl;
//         return;
//     }

//     string line;
//     while (getline(infile, line)) {
//         Book book;
//         book.loadFromFile(infile); // Use Book's load function to populate its fields

//         gBook[book.getISBN()] = book; // Add book to the map using its ISBN as key
//     }
//     cout<<"Books loaded from file Books.csv Successfully."<<endl;
//     infile.close();
// }
void Library::loadBooksFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string isbn, title, author, publisher, yearStr, status;

        // Parse fields from the CSV line
        getline(ss, isbn, ',');
        getline(ss, title, ',');
        getline(ss, author, ',');
        getline(ss, publisher, ',');
        getline(ss, yearStr, ',');
        getline(ss, status, ',');

        // Validate fields
        if (isbn.empty() || title.empty() || author.empty() || publisher.empty() || yearStr.empty() || status.empty()) {
            cerr << "Warning: Skipping invalid book entry in file: " << line << endl;
            continue; // Skip invalid rows
        }

        // Convert year from string to integer
        int year;
        try {
            year = stoi(yearStr); // Convert year to integer
        } catch (const invalid_argument& e) {
            cerr << "Warning: Invalid year format for book with ISBN " << isbn << ". Skipping entry." << endl;
            continue; // Skip rows with invalid year format
        }

        // Create a Book object and add it to the map
        Book book(title, author, publisher, year, isbn, status);
        gBook[isbn] = book; // Add book to the global map using ISBN as key
    }

    cout << "Books loaded from file " << filename << " successfully." << endl;
    infile.close();
}

void Library::saveBooksToFile(const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    for (const auto& pair : gBook) {
        pair.second.saveToFile(outfile); // Use Book's save function to write its fields
    }
    cout<<"Books saved to file Books.csv Successfully."<<endl;
    outfile.close();
}




void Library::loadAccountsFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        Account account;
        account.loadFromFile(infile); // Use accounts's load function to populate its fields

        gAccount[account.getUserID()] = account; // Add account to the map using its userID as key
    }
    cout<<"Accounts loaded from file Accounts.csv Successfully."<<endl;
    infile.close();
}
// void Library::loadAccountsFromFile(const string& filename) {
//     ifstream infile(filename);
    
//     if (!infile.is_open()) {
//         cerr << "Error: Could not open file " << filename << endl;
//         return;
//     }

//     while (!infile.eof()) { // Loop until end-of-file
//         Account account;
        
//         account.loadFromFile(infile); // Use Account's load function to populate its fields
        
//         if (!account.getUserID().empty()) { // Ensure valid account before adding to map
//             gAccount[account.getUserID()] = account; // Add account to the global map using userID as key
//         } else {
//             cerr << "Warning: Skipping invalid account entry.\n";
//         }
        
//         // Clear any remaining errors in the stream to avoid infinite loops
//         infile.clear();
//     }

//     cout << "Accounts loaded from file " << filename << " successfully." << endl;
// }


void Library::saveAccountsToFile(const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    for (const auto& pair : gAccount) {
        pair.second.saveToFile(outfile); // Use Accounts's save function to write its fields
    }
    cout<<"Accounts saved to file Accounts.csv Successfully."<<endl;
    outfile.close();         
        
}
//===========================================END OF CLASSESS============================================


int main() {
    cout<<"========================================================\n";
    cout << "======= Welcome to the Library Management System =======\n";
    cout<<"========================================================\n";

    // Initialize Library
    Library library;

    // Load existing data or initialize default data
    cout << "\nLoading library data...\n";
    library.loadLibrary()    ;

    if (gStudent.empty() && gFaculty.empty() && gLibrarian.empty()) {
        cout << "No existing users found. Initializing default data...\n";
        //default data
        // Add a librarian
        Librarian librarian("Librarian1", "lib001", "libpass");
        librarian.addUser("Librarian1", "lib001", "libpass","Librarian");
        // Add 10 sample books
        librarian.addBook("The C Programming Language", "Brian W. Kernighan", "Prentice Hall", 1978, "978-0131103627", "available");
        librarian.addBook("Design Patterns: Elements of Reusable Object-Oriented Software", "Erich Gamma", "Addison-Wesley", 1994, "978-0201633610", "available");
        librarian.addBook("Clean Code: A Handbook of Agile Software Craftsmanship", "Robert C. Martin", "Prentice Hall", 2008, "978-0132350884", "available");
        librarian.addBook("Head First Design Patterns", "Eric Freeman", "O'Reilly Media", 2004, "978-0596009205", "available");
        librarian.addBook("Effective Modern C++", "Scott Meyers", "O'Reilly Media", 2014, "978-0134494166", "available");
        librarian.addBook("Fluent Python: Clear, Concise, and Effective Programming", "Luciano Ramalho", "O'Reilly Media", 2015, "978-1491950357", "available");
        librarian.addBook("Introduction to Algorithms", "Thomas H. Cormen", "MIT Press", 2009, "978-0134685991", "available");
        librarian.addBook("Artificial Intelligence: A Modern Approach", "Stuart Russell & Peter Norvig", "Pearson Education", 2010, "978-1118008188", "available");
        librarian.addBook("Operating System Concepts", "Abraham Silberschatz, Peter B. Galvin, Greg Gagne", "Wiley", 2012, "978-0131101630", "available");
        librarian.addBook("The Art of Computer Programming (Volume 1)", "Donald Knuth", "Addison-Wesley Professional", 1997, "978-0321573513", "available");               
        // Add 3 faculty members
        librarian.addUser("Dr. Alice Smith", "fac001", "facpass1","Faculty");
        librarian.addUser("Dr. Bob Johnson", "fac002", "facpass2","Faculty");
        librarian.addUser("Dr. Carol Davis", "fac003", "facpass3","Faculty");

        // Add 5 students
        librarian.addUser("John Doe", "stu001", "stupass1", "Student");
        librarian.addUser("Jane Roe", "stu002", "stupass2", "Student");
        librarian.addUser("Mike Lee", "stu003", "stupass3", "Student");
        librarian.addUser("Emily Clark", "stu004", "stupass4", "Student");
        librarian.addUser("Chris Evans", "stu005", "stupass5", "Student");
        cout << "\nDefault data initialized successfully.\n";
    }
    

    // Clear input buffer after initialization
    cin.clear();
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');
    bool exitSystem = false;

    while (!exitSystem) {
        cout << "\n=== Main Menu ===\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: \n";
        int choice;
        cin>>choice;
        switch (choice) {
            case 1: {
                string userID, password;
                cout << "\nEnter User ID: ";
                cin >> userID;
                cout << "Enter Password: ";
                cin >> password;

                User* currentUser = nullptr;

                // Authenticate user
                if (gStudent.find(userID) != gStudent.end()) {
                    currentUser = &gStudent[userID];
                    cout<<"Authentication Succesfull!\n";
                } else if (gFaculty.find(userID) != gFaculty.end()) {
                    currentUser = &gFaculty[userID];
                    cout<<"Authentication Succesfull!\n";
                } else if (gLibrarian.find(userID) != gLibrarian.end()) {
                    currentUser = &gLibrarian[userID];
                    cout<<"Authentication Succesfull!\n";
                } else{
                    cout<<"You are not a registered user , PLease register first"<<endl;
                    break;
                }
                

                if (currentUser && currentUser->getPassword() == password) {
                    cout << "\nLogin successful! Welcome, " << currentUser->getName() << ".\n";

                    bool logout = false;
                    while (!logout) {
                        cout << "\n=== User Menu ===\n";
                        
                        if (dynamic_cast<Student*>(currentUser)) { //Type Checking(It checks if the object pointed to by the currentUser 
                                                                   //pointer is of type Student or is derived from the Student class.)
                            cout << "1. View available Books\n";
                            cout << "2. Borrow Book\n";
                            cout << "3. Return Book\n";
                            cout << "4. View Borrowed Books\n";
                            cout << "5. View Fine Details Or Veiw Account Details\n";
                            cout << "6. View Borrowing History\n";
                            cout << "7. Pay Fine\n";
                            cout<<  "8. Denroll from the Library Management System\n";
                            cout<<  "9. Know Total Fine and Book wise Fine\n";
                            cout << "10. Logout\n";
                        } else if (dynamic_cast<Faculty*>(currentUser)) {
                            cout << "1. View available Books\n";
                            cout << "2. Borrow Book\n";
                            cout << "3. Return Book\n";
                            cout << "4. View Borrowed Books\n";
                            cout << "5. View Account Details\n";
                            cout << "6. View Borrowing History\n";
                            cout << "7. Logout\n";
                            cout<<  "8. Denroll from the Library Management System\n";
                        } else if (dynamic_cast<Librarian*>(currentUser)) {
                            cout << "1. Add User\n";
                            cout << "2. Remove User\n";
                            cout << "3. Add Book\n";
                            cout << "4. Remove Book\n";
                            cout << "5. Update Book Details\n";
                            cout << "6. View All Books\n";
                            cout << "7. View All Users\n";
                            cout<<  "8. Denroll from the Library Management System\n";
                            cout << "9. Logout\n";
                        }
                        cout<<"=> Choose corresponding integer to the requests shown above:";
                        int userChoice;
                        if (!(cin >> userChoice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a number.\n";
                            continue;
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer after input
                        int currentDate = getCurrentDateInSeconds();

                        switch (userChoice) {
                            case 1:
                            {    if (dynamic_cast<Librarian*>(currentUser)) {
                                    string name, id, password, role;
                                    cout << "\nEnter User Details:\nName: ";
                                    // cin.ignore();
                                    cin>>name;
                                    cout << "ID: ";
                                    cin >> id;
                                    cout << "Password: ";
                                    cin >> password;
                                    cout << "Role (Student/Faculty/Librarian): ";
                                    cin >> role;
                                    dynamic_cast<Librarian*>(currentUser)->addUser(name, id, password, role);
                                } else {
                                    library.dispalyAllBooks();
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 2:
                            {    if (dynamic_cast<Librarian*>(currentUser)) {
                                    string id,role;
                                    cout << "\nEnter User ID to Remove: ";
                                    cin >> id;
                                    cout<<"\n Enter Role(Student/Faculty/Librarian):";
                                    cin>> role;
                                    dynamic_cast<Librarian*>(currentUser)->removeUser(id,role);
                                } else if(dynamic_cast<Student*>(currentUser)){
                                    string isbn;
                                    cout << "\nEnter ISBN of the book to borrow: ";
                                    cin >> isbn;
                                    dynamic_cast<Student*>(currentUser)->borrowBook(isbn, currentDate);
                                }else{
                                    string isbn;
                                    cout << "\nEnter ISBN of the book to borrow: ";
                                    cin >> isbn;
                                    dynamic_cast<Faculty*>(currentUser)->borrowBook(isbn, currentDate);
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 3:
                            {    if (dynamic_cast<Librarian*>(currentUser)) { //add book
                                    string isbn, title, author, publisher;
                                    int year;
                                    string status;
                                
                                    // cin.ignore();
                                    cout << "\nEnter Book Details:\nISBN: ";
                                    getline(cin, isbn);
                                    cout << "Title: ";
                                    getline(cin, title);
                                    cout << "Author: ";
                                    getline(cin, author);
                                    cout << "Publisher: ";
                                    getline(cin, publisher);
                                    
                                    cout << "Year: ";
                                    cin >> year;
                                
                                    cout << "\nIs this book reserved for someone (yes/no): ";
                                    cin>>status;
                                
                                    // Now check status INSIDE the same scope
                                    if (status == "yes" || status == "YES") {
                                        dynamic_cast<Librarian*>(currentUser)->addBook(title, author, publisher, year, isbn, "reserved");
                                        cout << "\nBook added successfully with status RESERVED.\n";
                                        
                                    } else if (status == "no" || status == "NO") {
                                        dynamic_cast<Librarian*>(currentUser)->addBook(title, author, publisher, year, isbn, "available");
                                        cout << "\nBook added successfully with status AVAILABLE.\n";
                                        
                                    } else {
                                        cout << "\nInvalid input for reservation status. Please enter 'yes' or 'no'.\n";
                                    }
                                }else { //return book
                                    string isbn;
                                    cout << "\nEnter ISBN of the book to return: ";
                                    cin >> isbn;
                                    currentUser->returnBook(isbn, currentDate);
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 4:
                            {    if (dynamic_cast<Librarian*>(currentUser)) { //remove book
                                    string isbn;
                                    cin.ignore();
                                    cout << "\nEnter Book Details:\nISBN: ";
                                    getline(cin, isbn);
                                    dynamic_cast<Librarian*>(currentUser)->removeBook(isbn);
                                } else{ //View Borrowed Books
                                    currentUser->account->showCurrentBorrowedBooks();
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 5:
                            {   if (dynamic_cast<Librarian*>(currentUser)) {    //update book
                                    string isbn, title, author, publisher;
                                    int year;
                                    string status;
                                
                                    cin.ignore();
                                    cout << "\nEnter Book Details:\n ISBN of the book to be Updated: ";
                                    getline(cin, isbn);
                                    cout << "Updated Title: ";
                                    getline(cin, title);
                                    cout << " Updated Author: ";
                                    getline(cin, author);
                                    cout << "Updated Publisher: ";
                                    getline(cin, publisher);
                                    
                                    cout << "Updated Year: ";
                                    cin >> year;
                                
                                    cout << "\nIs this book reserved for someone (yes/no): ";
                                    getline(cin,status);
                                
                                    // Now check status INSIDE the same scope
                                    if (status == "yes" || status == "YES") {
                                        dynamic_cast<Librarian*>(currentUser)->addBook(title, author, publisher, year, isbn, "reserved");
                                        cout << "\nBook Updated successfully with status RESERVED.\n";
                                        
                                    } else if (status == "no" || status == "NO") {
                                        dynamic_cast<Librarian*>(currentUser)->addBook(title, author, publisher, year, isbn, "available");
                                        cout << "\nBook Updated successfully with status AVAILABLE.\n";
                                        
                                    } else {
                                        cout << "\nInvalid input for reservation status. Please enter 'yes' or 'no'.\n";
                                    }
                                    
                                } else{
                                    cout<<"Your account details:\n";
                                    currentUser->account->displayAccountDetails(currentDate);      
                                    }
                                    cout<<"Reload User Menu..Enter digit\"1\":";
                                    int reload;
                                    cin>>reload;
                                    if(reload)
                                        break;
                            }

                            case 6:
                                {if(dynamic_cast<Librarian*>(currentUser)){ //View All Books
                                    library.dispalyAllBooks();
                                }
                                else{  //borrowing history
                                    currentUser->account->showBorrowHistory();
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }

                            case 7:
                                {if(dynamic_cast<Librarian*>(currentUser)){ //View All users
                                    library.viewAllUsers();
                                }else if(dynamic_cast<Student*>(currentUser)){ //pay fine
                                    string isbn;
                                    int temp;
                                    double amount;
                                    cout<<"1: pay fine for a perticular Book "<<endl;
                                    cout<<"2. Pay total Fine"<<endl;
                                    cout<<"=>Choose Integer corresponding to Payment Options:";
                                    cin>>temp;
                                    switch (temp)
                                    {
                                    case 1:
                                        cout<<"Enter the ISBN for the book you want to pay fine:";
                                        cin>>isbn;
                                        cout<<"\nEnter the exact amount to be paid for this Book:";
                                        cin>> amount;
                                        currentUser->account->payFineForBook(isbn,amount,currentDate);

                                    case 2:
                                        cout<<"Enter exact amount equal to total fine: ";
                                        cin>>amount;
                                        currentUser->account->payTotalFine(amount,currentDate);
                                        break;
                                    
                                    default:
                                        break;
                                    }
                                }
                                else{  //logout Faculty
                                    cout<<"Thanks for using our software."<<endl;
                                    cout <<"You are Loging Out ...\n";
                                    logout=true;
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 8: //denroll 
                             {
                                string role;
                                if(dynamic_cast<Student*>(currentUser))
                                    role="Student";
                                else if(dynamic_cast<Faculty*>(currentUser))
                                    role="Faculty";
                                else
                                    role="Librarian";
                                string temp;
                                cout<<"\nDo you want to leave library management system (yes/no)";
                                cin>>temp;
                                if(temp=="yes"|| temp=="YES"){
                                    gLibrarian["lib001"].removeUser(userID,role);
                                    cout<<"Happy to see you again.\n";
                                }
                                logout=true;
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                             }
                            case 9:
                            {
                                if(dynamic_cast<Student*>(currentUser)){
                                    cout<<"Your Total Fine:";
                                    double tfine=dynamic_cast<Student*>(currentUser)->account->getTotalFine();
                                    cout<<tfine;
                                    cout <<"\n Do you want to know fine for perticular book? (yes/no)\n";
                                    string temp;
                                    if(temp=="yes"|| temp=="YES"){
                                        cout<<"Enter the ISBN of book for which you need to know fine"<<endl;
                                        string isnbb;
                                        getline(cin,isnbb);
                                        double finnn=dynamic_cast<Student*>(currentUser)->account->getFineForBook(isnbb);
                                        cout<<"Fine for book ISNB :"<<isnbb<<" is "<<finnn<<".\n";

                                    }
                                }
                                else{
                                    logout=true;
                                }
                                cout<<"Reload User Menu..Enter digit\"1\":";
                                int reload;
                                cin>>reload;
                                if(reload)
                                    break;
                            }
                            case 10:{
                                logout=true;

                            }
                        }
                    }
                }else{
                    cout<<"Login insuccesfull!! , incorrect password\n";
                }
                break;
            }
            case 2:{
                cout<<"Enter details for registration:\nName:";
                string name ,userID, password,role;
                cin>> name;
                cout << "\nEnter User ID: ";
                cin >> userID;
                cout << "\nEnter Password: ";
                cin >> password;
                cout<<"\nEnter your role(Student/Faculty/Librarian)";
                cin>> role;
                gLibrarian["lib001"].addUser(name,userID,password,role);
                break;
            }
            case 3:{
                exitSystem=true;
                break;
            }   
        }
    }
    library.saveLibrary();
    return 0;
}



















                                             
