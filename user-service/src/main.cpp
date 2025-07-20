#include <iostream>
#include <httplib.h>
#include <string>
#include <memory>
#include "Database.h"

using namespace std;
using namespace httplib;

using json = nlohmann::json;


int main(){
    try{
        unique_ptr<Database> db = make_unique<Database>("user_db.db");

        // testing createUser() and getUser()
        // Test creating user
        int lUserId = db->createUser("divansh_kawatra", "divanshk@example.com", "password5$");
        cout << "Created user with ID: " << lUserId << endl;
        
        // Test getting user
        // int lUserId = 1;
        json user = db->getUser(lUserId);
        cout << "Retrieved user: " << user.dump(4) << endl;
    }
    catch(const invalid_argument& e){
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    catch(const exception& e){
        cerr<<"Caught Exception: "<<e.what()<<endl;
        return 1;
    }
    return 0;
}