
// Database.h
// Header Guards/Include Guard
#ifndef DATABASE_H  // Conditional block start: "If NOT defined DATABASE_H"
#define DATABASE_H  // Define DATABASE_H

#include <iostream>
#include <string>
#include <sqlite3.h>
#include <optional> // C++17 feature - 

// if a header file includes a using namespace directive or a using declaration at the global
// scope, that effect will be propagated to any .cpp file(or other header file) that includes it.
// **remove the using namespace std; from the header file. This is a best practice to avoid 
// polluting the namespace of any other file that includes it.
// It's better to explicitly use std::string, std::optional, etc., in header files.
// using namespace std;

struct User {
    int id;
    std::string username;
    std::string email;
    // string password; // we don't want to load sensitive data into memory when it's not needed.
    std::string created_at;
};

class Database {
    sqlite3* mDB;

    public:
    Database(const std::string dbname);

    ~Database();

    // void *data: This is the user-defined data passed as the fourth argument to sqlite3_exec. In this example, it's a string "Callback function called".
    // int argc: The number of columns in the current row.
    // char **argv: An array of strings, where argv[i] is the value of the i-th column.
    // char **azColName: An array of strings, where azColName[i] is the name of the i-th column.
    static int executeQueryCallback(void* data, int argc, char** argv, char** azcolNames);

    /// method to create the users table with fields: id, username, email, password, created_at
    void createTables();

    // function to create user
    int createUser(const std::string& pUsername, const std::string& pEmailId, const std::string& pPassword);

    // function to get user
    std::optional<User> getUserById(int pUserId);

    private:
    // function to validate email address format
    bool isValidEmail(const std::string& pEmailId);
};

#endif   // End of the Conditional Block