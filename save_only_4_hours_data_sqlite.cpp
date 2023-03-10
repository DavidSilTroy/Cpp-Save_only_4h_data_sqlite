#include <thread> // To can insert and delete data at the same time
#include <iostream> // input and output functionality. Print in terminal.
#include <iomanip> // function to manipulate the output of the '<iostream>'.
#include <sstream> // for working with strings as streams. Used to format the date and time string
#include <chrono> // for real-time operations.
#include <string> // basic string manipulation functions.
#include <sqlite3.h> // the database library


using namespace std;

// SQLite database filename
const char* DB_FILE = "data_to_try_four_hours_save.db";

// SQLite database table name
const char* TABLE_NAME = "input_data_table";

// Function to create database table
int create_table(sqlite3* db)
{
    // SQL query to create table
    const char* query = "CREATE TABLE IF NOT EXISTS input_data_table (timestamp INTEGER PRIMARY KEY, data TEXT, data_time TEXT);";

    // Execute query
    int rc = sqlite3_exec(db, query, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        cerr << "Error creating table: " << sqlite3_errmsg(db) << endl;
        return rc;
    }

    cout << "Table created successfully" << endl;

    return SQLITE_OK;
}

int main()
{
    // Open database connection
    sqlite3* db;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return rc;
    }

    // Create table if it does not exist
    rc = create_table(db);

    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return rc;
    }

    // Get current time
    auto now = chrono::system_clock::now();

    // Loop to insert data every millisecond
    while (true) {
        // Get current time
        now = chrono::system_clock::now();

        // Convert time to milliseconds
        auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        
        // Convert to dd/mm/yyyy - hh:mm:ss
        time_t current_time = chrono::system_clock::to_time_t(now);  

		// Format date and time as string
		stringstream ss;
		ss << put_time(localtime(&current_time), "%d/%m/%Y - %T");
		string formatted_time = ss.str();

        // Generate input data
        string data = "input data " + to_string(timestamp);

        // SQL query to insert data
        string query = "INSERT INTO input_data_table (timestamp, data, data_time) VALUES (" + to_string(timestamp) + ", '" + data + "', '" + formatted_time + "');";

        // Execute query
        rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);

        if (rc != SQLITE_OK) {
            cerr << "Error inserting data: " << sqlite3_errmsg(db) << endl;
        } else {
            cout << "Data inserted successfully: " << data << endl;
            cout << "with the time: " << formatted_time << endl;
        }

        // Sleep for 1 millisecond
        this_thread::sleep_for(chrono::milliseconds(1));

        // Delete data older than 4 hours
        auto four_hours_ago = now - chrono::hours(4);
        auto timestamp_4_hours_ago = chrono::duration_cast<chrono::milliseconds>(four_hours_ago.time_since_epoch()).count();
        query = "DELETE FROM input_data_table WHERE timestamp < " + to_string(timestamp_4_hours_ago) + ";";
        rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);

        if (rc != SQLITE_OK) {
            cerr << "Error deleting data: " << sqlite3_errmsg(db) << endl;
        }
    }

    // Close database connection
    sqlite3_close(db);

    return 0;
}
