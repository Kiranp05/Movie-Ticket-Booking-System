#include <iostream>
#include <mysql.h>
#include <windows.h>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <vector>

using namespace std;

const char* HOST = "localhost";
const char* USER = "root";
const char* PW = "kiranp7010";
const char* DB = "db2";

class Seats {
private:
    int Seat[22][22];

public:
    Seats() {
        for (int i = 0; i < 22; i++) {
            for (int j = 0; j < 22; j++) {
                Seat[i][j] = 1;
            }
        }
    }
    
    string generateBookingId() const {
        const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const int idLength = 8;
        string bookingId;

        srand(static_cast<unsigned int>(time(0)));
        for (int i = 0; i < idLength; ++i) {
            bookingId += chars[rand() % chars.length()];
        }

        return bookingId;
    }

    int getSeatStatus(int row, int seatNumber) {
        if (row < 1 || row > 22 || seatNumber < 1 || seatNumber > 22) {
            return -1;
        }
        return Seat[row - 1][seatNumber - 1];
    }

    void reserveSeat(int row, int seatNumber) {
        if (row < 1 || row > 22 || seatNumber < 1 || seatNumber > 22) {
            return;
        }
        Seat[row - 1][seatNumber - 1] = 0;
    }

    void display() {
        for (int row = 0; row < 22; row++) {
            for (int col = 0; col < 22; col++) {
                if (Seat[row][col] == 1) {
                    cout << char('A' + row) << col + 1 << " ";
                } else {
                    cout << "X ";
                }
            }
            cout << "|" << endl;
        }
        cout << "------------------------------------------------------" << endl;
    }

    void getDB(MYSQL* conn) {
        string query = "SELECT RowNumber, SeatNumber, Seat FROM Ticket";
        if (mysql_query(conn, query.c_str())) {
            cout << "Error 1: " << mysql_error(conn) << endl;
        }

        MYSQL_RES* result;
        result = mysql_store_result(conn);
        if (!result) {
            cout << "Error 2: " << mysql_error(conn) << endl;
        }
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            int rowNumber = atoi(row[0]);
            int seatNumber = atoi(row[1]);
            int seatStatus = atoi(row[2]);
            Seat[rowNumber - 1][seatNumber - 1] = seatStatus;
        }
        mysql_free_result(result);
    }

    void resetAllBookings(MYSQL* conn) {
        for (int row = 1; row <= 22; row++) {
            for (int seatNumber = 1; seatNumber <= 22; seatNumber++) {
                stringstream ss;
                ss << "UPDATE Ticket SET Seat = 1 WHERE RowNumber = " << row << " AND SeatNumber =" << seatNumber;
                string update = ss.str();
                if (mysql_query(conn, update.c_str())) {
                    cout << "Error: " << mysql_error(conn) << endl;
                }
                Seat[row - 1][seatNumber - 1] = 1;
            }
        }
        cout << "All bookings have been reset." << endl;
    }

    void displayBookedTickets() const {
    cout << "Total Booked Tickets: ";
    int bookedCount = 0;
    for (int row = 0; row < 22; row++) {
        for (int col = 0; col < 22; col++) {
            if (Seat[row][col] == 0) {
                cout << char('A' + row) << col + 1 << " ";
                bookedCount++;
            }
        }
    }
    cout << endl;

    cout << "Number of Booked Tickets: " << bookedCount << endl;
}

};

class Movies {
public:
    MYSQL* conn;
    Movies(MYSQL* connection) : conn(connection) {}

    void addMovie() {
        string queryCheck = "SELECT COUNT(*) FROM Movies";
        if (mysql_query(conn, queryCheck.c_str())) {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_RES* resultCheck;
        resultCheck = mysql_store_result(conn);
        if (!resultCheck) {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW rowCheck = mysql_fetch_row(resultCheck);
        int movieCount = atoi(rowCheck[0]);

        mysql_free_result(resultCheck);

        if (movieCount > 0) {
            cout << "There is already a movie running. Cannot add another movie at this time." << endl;
            return;
        }

        string movieName;
        cout << "Enter the movie name: ";
        cin.ignore(); // Clear newline from previous input
        getline(cin, movieName);

        string query = "INSERT INTO Movies (MovieName) VALUES ('" + movieName + "')";
        if (mysql_query(conn, query.c_str())) {
            cout << "Error: " << mysql_error(conn) << endl;
        }
    }

    void deleteMovie() {
        string movieName;
        cout << "Enter the name of the movie to delete: ";
        cin.ignore(); // Clear newline from previous input
        getline(cin, movieName);

        string query = "DELETE FROM Movies WHERE MovieName = '" + movieName + "'";
        if (mysql_query(conn, query.c_str())) {
            cout << "Error: " << mysql_error(conn) << endl;
        }
    }

    void displayMovies() {
        string query = "SELECT MovieName FROM Movies";
        if (mysql_query(conn, query.c_str())) {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_RES* result;
        result = mysql_store_result(conn);
        if (!result) {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW row;
        cout << "Movies Showing Now:" << endl;
        while ((row = mysql_fetch_row(result))) {
            cout << row[0] << endl;
        }
        cout << "--------------------------" << endl;

        mysql_free_result(result);
    }
};

void displayMenu() {
    cout << endl;
    cout << "Welcome To SPR CINECASTLE" << endl;
    cout << "*************************" << endl;
    cout << "1. Movies Showing Now!" << endl;
    cout << "2. Reserve Tickets" << endl;
    cout << "3. Total Booked Tickets" << endl;
    cout << "4. Reset All Bookings" << endl;
    cout << "5. Add Movies" << endl;
    cout << "6. Delete Movies" << endl;
    cout << "7. Exit" << endl;
    cout << "Enter Your Choice: ";
}

bool loginUser(MYSQL* conn) {
    string username, password;

    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;

    string query = "SELECT * FROM Users WHERE Username='" + username + "' AND Password='" + password + "'";

    if (mysql_query(conn, query.c_str())) {
        cout << "Error: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES* result;
    result = mysql_store_result(conn);
    if (!result) {
        cout << "Error: " << mysql_error(conn) << endl;
        return false;
    }

    int numRows = mysql_num_rows(result);

    mysql_free_result(result);

    if (numRows == 1) {
        cout << "Login Successful!" << endl;
        return true;
    } else {
        cout << "Wrong Username/Password. Please try again." << endl;
        return false;
    }
}

int main() {
    MYSQL* conn;
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, HOST, USER, PW, DB, 3306, NULL, 0)) {
        cout << "Error: " << mysql_error(conn) << endl;
        return 1;
    } else {
        cout << "Logged In Database!" << endl;
    }

    int loginAttempts = 3;
    bool loggedIn = false;

    while (loginAttempts > 0) {
        loggedIn = loginUser(conn);

        if (loggedIn) {
            break;
        } else {
            loginAttempts--;
            cout << "Remaining login attempts: " << loginAttempts << endl;
        }
    }

    if (!loggedIn) {
        cout << "Too many login attempts. Exiting program." << endl;
        mysql_close(conn);
        return 1;
    }

    Seats s;
    Movies movieSystem(conn);

    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS Movies (MovieName VARCHAR(255))")) {
        cout << "Error: " << mysql_error(conn) << endl;
        return 1;
    }

    bool exit = false;
    while (!exit) {
        displayMenu();
        int val;
        cin >> val;

        switch (val) {
            case 1: {
                movieSystem.displayMovies();
                cout << "Press any key to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 2: {
			    string selectedMovie = "default_movie";
			
			    s.getDB(conn);
			    s.display();
			
			    int numTickets;
			    cout << "Enter the number of tickets to reserve: ";
			    cin >> numTickets;
			
			    int totalCost = 0;
			    vector<string> bookedSeats;
			
			    for (int i = 0; i < numTickets; i++) {
			        string seatLocation;
			        cout << "Enter Seat Location (e.g., A1, B2) for Ticket " << i + 1 << ": ";
			        cin >> seatLocation;
			
			        char row = seatLocation[0];
			        int col = atoi(seatLocation.substr(1).c_str());
			
			        int rowNumber = row - 'A' + 1;
			
			        if (rowNumber < 1 || rowNumber > 22 || col < 1 || col > 22 || s.getSeatStatus(rowNumber, col) == -1 || s.getSeatStatus(rowNumber, col) == 0) {
			            cout << "Invalid Seat Location for Ticket " << i + 1 << "! Try again." << endl;
			            i--;
			            continue;
			        }
			
			        s.reserveSeat(rowNumber, col);
			        stringstream ss;
			        ss << "UPDATE Ticket SET Seat = 0 WHERE RowNumber = " << rowNumber << " AND SeatNumber =" << col;
			        string update = ss.str();
			        if (mysql_query(conn, update.c_str())) {
			            cout << "Error: " << mysql_error(conn) << endl;
			        } else {
			            int ticketPrice = (row >= 'A' && row <= 'B') ? 60 : ((row >= 'C' && row <= 'V') ? 190 : 0);
			            totalCost += ticketPrice;
			            bookedSeats.push_back(seatLocation);
			            cout << "Ticket " << i + 1 << " Reserved Successfully" << endl;
			        }
			    }
			
			    // Display the list of booked seats
			    cout << "Tickets booked: ";
			    for (vector<string>::const_iterator it = bookedSeats.begin(); it != bookedSeats.end(); ++it) {
				    cout << *it;
				    if (it + 1 != bookedSeats.end()) {
				        cout << ",";
				    }
				}
				cout << endl;
			
			    // Generate and display the booking ID
			    string bookingId = s.generateBookingId();
			    cout << "Booking ID: " << bookingId << endl;
			
			    cout << "Total Cost: Rs" << totalCost << endl;
			    cout << "--------------------------" << endl;
			    cout << "Press any key to continue...";
			    cin.ignore();
			    cin.get();
			    break;
			}
            case 3: {
                s.displayBookedTickets();
                cout << "Press any key to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 4: {
                s.resetAllBookings(conn);
                cout << "Press any key to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 5: {
                movieSystem.addMovie();
                cout << "Movie added successfully!" << endl;
                cout << "Press any key to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 6: {
                movieSystem.deleteMovie();
                cout << "Movie deleted successfully!" << endl;
                cout << "Press any key to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 7:
                exit = true;
                cout << "Exit" << endl;
                Sleep(3000);
                break;
            default:
                cout << "Invalid Input" << endl;
                Sleep(3000);
        }
    }

    mysql_close(conn);
    return 0;
}
