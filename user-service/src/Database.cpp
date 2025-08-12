#include "Database.h"
// #include <exception>
#include <stdexcept> // for invalid_argument, and other exceptions
#include <regex>    // Required for regex functionality

using namespace std;

Database::Database(const string dbname){
    cout<<"Database constructor called !"<<endl;
    
    // Opens SQLite connection in constructor
    int rc = sqlite3_open(dbname.c_str(), &mDB);
    if(rc != SQLITE_OK) {   // if(rc){
        // If we only output error, the program will continue to run; so better approach is to 
        // throw an error
        // cerr<<"Can't open database user_db.db"<<sqlite3_errmsg(mDB)<<endl;
        throw runtime_error("Error opening DB: " + string(sqlite3_errmsg(mDB)));
        return;
    }

    createTables();
}

Database::~Database(){
    // Closes connection in destructor
    sqlite3_close(mDB);
}


// void *data: This is the user-defined data passed as the fourth argument to sqlite3_exec. In this example, it's a string "Callback function called".
// int argc: The number of columns in the current row.
// char **argv: An array of strings, where argv[i] is the value of the i-th column.
// char **azColName: An array of strings, where azColName[i] is the name of the i-th column.
int Database::executeQueryCallback(void* data, int argc, char** argv, char** azcolNames){
    cout<<"argument passed from sqlite3_exec():"<<(char*)data<<endl;
    for(int i = 0; i<argc; ++i){
        cout<<azcolNames[i]<<": "<<argv[i]<<" ";
    }
    cout<<endl;
}

/// method to create the users table with fields: id, username, email, password, created_at
void Database::createTables(){
    cout<<"Executing create table command..."<<endl;
    string lCreateTableCommand = "CREATE TABLE IF NOT EXISTS users (\
                                    id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                    username TEXT NOT NULL,\
                                    email TEXT NOT NULL UNIQUE,\
                                    password TEXT NOT NULL,\
                                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP\
                                    )";
    char* errMsg = nullptr;
    // char* callbackData = "<data from exec()>";
    // int rc = sqlite3_exec(mDB, lCreateTableCommand.c_str(), executeQueryCallback, callbackData, &errMsg);
    // Callback is only needed for SELECT queries, no need here
    int rc = sqlite3_exec(mDB, lCreateTableCommand.c_str(), nullptr, nullptr, &errMsg);
    if(rc){ // i.e. rc != SQLITE_OK
        cerr<<"Error creating table: "<<errMsg<<endl;
        sqlite3_free(errMsg); // **** release errMsg to prevent memory leak !! ****
        return;
    }
    cout<<"Table created successfully!"<<endl;
    return;
}

// function to create user
int Database::createUser(const string& pUsername, const string& pEmailId, const string& pPassword){
    // first validate few things
    // 1. email is of the form *@*.com
    // 2. a user with same email id does not already exist
    if(!isValidEmail(pEmailId)){
        throw invalid_argument("Invalid Email Format! Required email format: *@*.*");
    }

    string lQuery = "INSERT INTO users (username, email, password) VALUES (?, ?, ?);";
    sqlite3_stmt* lPreparedStmt;
    int rc = sqlite3_prepare_v2(mDB, lQuery.c_str(), -1, &lPreparedStmt, nullptr);
    if(rc != SQLITE_OK){
        throw runtime_error("Error while creating PreparedStatement: " + string(sqlite3_errmsg(mDB)));
    }

    // bind values for column data
    rc = sqlite3_bind_text(lPreparedStmt, 1, pUsername.c_str(), -1, SQLITE_TRANSIENT);
    if(rc != SQLITE_OK){
        throw runtime_error("createUser: Error while binding data to prepared statement");
    }
    rc = sqlite3_bind_text(lPreparedStmt, 2, pEmailId.c_str(), -1, SQLITE_TRANSIENT);
    if(rc != SQLITE_OK){
        throw runtime_error("createUser: Error while binding data to prepared statement");
    }
    
    // TODO: Hash password before storing (use bcrypt or argon2)
    rc = sqlite3_bind_text(lPreparedStmt, 3, pPassword.c_str(), -1, SQLITE_TRANSIENT);
    if(rc != SQLITE_OK){
        throw runtime_error("createUser: Error while binding data to prepared statement");
    }

    rc = sqlite3_step(lPreparedStmt);
    if(rc != SQLITE_DONE){
        sqlite3_finalize(lPreparedStmt);  // Destroys the prepared statement.
        // error condition
        if(rc == SQLITE_CONSTRAINT){
            throw runtime_error("Entered Email Id is already registered.");
        }
        else{
            throw runtime_error("Error while INSERT: " + string(sqlite3_errmsg(mDB)));
        }
    }

    sqlite3_finalize(lPreparedStmt);   // Destroys the prepared statement. 

    // get the user id of the last inserted user
    int lUserId = sqlite3_last_insert_rowid(mDB);

    return lUserId;
}

// function to get user
optional<User> Database::getUserById(int pUserId){
    string lQuery = "SELECT id, username, email, created_at FROM users WHERE id = ?";
    sqlite3_stmt* lPreparedStmt;
    int rc = sqlite3_prepare_v2(mDB, lQuery.c_str(), -1, &lPreparedStmt, nullptr);
    if(rc != SQLITE_OK){
        throw runtime_error("Error while creating PreparedStatement: " + string(sqlite3_errmsg(mDB)));
    }

    rc = sqlite3_bind_int(lPreparedStmt, 1, pUserId);
    if(rc != SQLITE_OK){
        throw runtime_error("getUserById: Error while binding data to prepared statement");
    }

    optional<User> lUserData = std::nullopt;
    rc = sqlite3_step(lPreparedStmt);
    if(rc == SQLITE_ROW){
        int lId = sqlite3_column_int(lPreparedStmt, 0);
        string lUsername = string((const char*)sqlite3_column_text(lPreparedStmt, 1));
        string lEmailId = string((const char*)sqlite3_column_text(lPreparedStmt, 2));
        // string lPassword = string((const char*)sqlite3_column_text(lPreparedStmt, 3));
        string lCreateDate = string((const char*)sqlite3_column_text(lPreparedStmt, 3));

        User lUser;
        lUser.id = lId;
        lUser.username = lUsername;
        lUser.email = lEmailId;
        lUser.created_at = lCreateDate;

        lUserData = lUser;
    }

    sqlite3_finalize(lPreparedStmt);
    return lUserData;
}


/////////////////// Helper Functions /////////////////////
// function to validate email address format
bool Database::isValidEmail(const string& pEmailId){
    // Regex pattern for "*@*.*" format
        // - ^: Start of the string
        // - [a-zA-Z0-9._%+-]+: One or more alphanumeric characters, '.', '_', '%', '+', or '-' 
        //       - I am allowing only dot(.) and underscor(_)
        // - @: The '@' symbol
        // - [a-zA-Z0-9.-]+: One or more alphanumeric characters, '.', or '-' for the domain
        // - \\.: A literal dot (escaped with double backslashes)
        // - com: The "com" top-level domain OR [a-zA-Z]{2,}: Matches the Top-Level Domain (TLD) part. It requires at least two (and potentially more) letters.
        // - $: End of the string
    // const regex pattern("^[a-zA-z0-9._]+@[]+\\.com$");    // for *@*.com
    const regex pattern("^[a-zA-z0-9._]+@[a-zA-Z0-9-]+\\.[a-zA-Z]{2,}$"); // for *@*.*

    return regex_match(pEmailId, pattern);
}

