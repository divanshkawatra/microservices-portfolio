
// Database.h
// Header Guards/Include Guard
#ifndef DATABASE_H.  // Conditional block start: "If NOT defined DATABASE_H"
#define DATABASE_H.  // Define DATABASE_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

// if a header file includes a using namespace directive or a using declaration at the global
// scope, that effect will be propagated to any .cpp file(or other header file) that includes it.
using namespace std;
using json = nlohmann::json;

class Database {
    sqlite3* mDB;

    public:
    Database(const string dbname);

    ~Database();

    // void *data: This is the user-defined data passed as the fourth argument to sqlite3_exec. In this example, it's a string "Callback function called".
    // int argc: The number of columns in the current row.
    // char **argv: An array of strings, where argv[i] is the value of the i-th column.
    // char **azColName: An array of strings, where azColName[i] is the name of the i-th column.
    static int executeQueryCallback(void* data, int argc, char** argv, char** azcolNames);

    /// method to create the users table with fields: id, username, email, password, created_at
    void createTables();

    // function to create user
    int createUser(const string& pUsername, const string& pEmailId, const string& pPassword);

    // function to get user
    json getUser(int pUserId);

    private:
    // function to validate email address format
    bool isValidEmail(const string& pEmailId);
};

#endif.   // End of the Conditional Block