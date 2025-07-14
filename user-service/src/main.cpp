#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <string>
#include <memory>
#include <exception>

using namespace std;
using namespace httplib;

using json = nlohmann::json;

/*
Database class that:
Opens SQLite connection in constructor
Closes connection in destructor
Has a createTables() method that creates the users table with these fields:
id (INTEGER PRIMARY KEY AUTOINCREMENT)
username (TEXT NOT NULL UNIQUE)
email (TEXT NOT NULL)
password (TEXT NOT NULL)
created_at (DATETIME DEFAULT CURRENT_TIMESTAMP)*/
class Database {
    sqlite3* mDB;

    public:
    Database(const string dbname){
        cout<<"Database constructor called !"<<endl;
        
        int rc = sqlite3_open(dbname.c_str(), &mDB);
        if(rc){
            // If we only output error, the program will continue to run; so better approach is to 
            // throw an error
            // cerr<<"Can't open database user_db.db"<<sqlite3_errmsg(mDB)<<endl;
            throw runtime_error("Error opening DB: " + string(sqlite3_errmsg(mDB)));
            return;
        }

        createTables();
    }

    ~Database(){
        sqlite3_close(mDB);
    }

    // void *data: This is the user-defined data passed as the fourth argument to sqlite3_exec. In this example, it's a string "Callback function called".
    // int argc: The number of columns in the current row.
    // char **argv: An array of strings, where argv[i] is the value of the i-th column.
    // char **azColName: An array of strings, where azColName[i] is the name of the i-th column.
    static int executeQueryCallback(void* data, int argc, char** argv, char** azcolNames){
        cout<<"argument passed from sqlite3_exec():"<<(char*)data<<endl;
        for(int i = 0; i<argc; ++i){
            cout<<azcolNames[i]<<": "<<argv[i]<<" ";
        }
        cout<<endl;
    }

    void createTables(){
        cout<<"Executing create table command..."<<endl;
        string lCreateTableCommand = "CREATE TABLE IF NOT EXISTS users (\
                                        id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                        username TEXT NOT NULL,\
                                        email TEXT NOT NULL UNIQUE,\
                                        password TEXT NOT NULL,\
                                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP\
                                     )";
        char* errMsg = nullptr;
        char* callbackData = "<data from exec()>";
        // int rc = sqlite3_exec(mDB, lCreateTableCommand.c_str(), executeQueryCallback, callbackData, &errMsg);
        // Callback is only needed for SELECT queries, no need here
        int rc = sqlite3_exec(mDB, lCreateTableCommand.c_str(), nullptr, nullptr, &errMsg);
        if(rc){
            cerr<<"Error creating table: "<<errMsg<<endl;
            sqlite3_free(errMsg); // **** release errMsg to prevent memory leak !! ****
            return;
        }
        cout<<"Table created successfully!"<<endl;
        return;
    }
};

int main(){
    try{
        unique_ptr<Database> db = make_unique<Database>("user_db.db");
    }
    catch(const exception& e){
        cerr<<"Caught Exception: "<<e.what()<<endl;
    }
    return 0;
}