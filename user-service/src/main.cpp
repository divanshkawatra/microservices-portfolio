#include<iostream>
#include<httplib.h>
#include<nlohmann/json.hpp>
#include<sqlite3.h>
#include<string>

using namespace std;
using namespace httplib;

using json = nlohmann::json;

int main() {
    // Test SQLite3
    sqlite3 *db;
    int rc = sqlite3_open("test.db", &db);
    if(rc){
        cerr<<"Can't open database: "<<sqlite3_errmsg(db)<<endl;
        return 1;
    }
    else{
        cout<<"SQLite working !"<<endl;
        sqlite3_close(db);
    }

    // Test JSON
    json j = {
        {"name", "John Doe"},
        {"age", 30},
        {"city", "New York"}
    };
    cout<<"JSON working: "<<j.dump(4)<<endl; // here, dump(4) formats the JSON with an indent of 4 spaces

    // Test HTTP server
    Server svr;
    svr.Get("/", [](const Request& req, Response& res){
        json response_json = {
            {"message", "Server is running"},
            {"status", "success"}
        };
        res.set_content(response_json.dump(4), "application/json");
    });

    cout<<"Starting HTTP server on port http://localhost:8001"<<endl;
    cout<<"Press Ctrl+C to stop the server."<<endl;

    svr.listen("localhost", 8001);

    return 0;
}

    