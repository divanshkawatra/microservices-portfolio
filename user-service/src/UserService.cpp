#include <iostream>
#include <nlohmann/json.hpp>
#include "UserService.h"

using json = nlohmann::json;

UserService::UserService(const string& pDBPath){
    mDatabaseObj = make_unique<Database>(pDBPath);
}

void UserService::setupRoutes(Server& pServer){
    pServer.Get("/health", [this](const Request& req, Response& res){
        this->handleHealthCall(req, res);
    });

    pServer.Post("/users", [this](const Request& req, Response& res){
        this->handleCreateUser(req, res);
    });

    // Regex Pattern breakdown
    // R - Raw string - no escaping needed: [e.g. without R: "/users/(\\d+)" ; with R: R"/users/(\\d+)"]
    // (  → Start capture group
    // /users/ → Literal text, must match exactly
    // \d → Matches any digit (0-9)
    // +  → One or more of the previous pattern
    // )  → End capture group
    pServer.Get(R"(/users/(\d+))", [this](const Request& req, Response& res){
        this->handleGetUser(req, res);
    });
}


// ************callback functions for REST calls***************
void UserService::handleHealthCall(const Request& req, Response& res){
    json lJson = {
        {"message", "User-Service is running."},
        {"status", "SUCCESS"},
        {"timestamp", time(nullptr)}
    };
    res.status = 200;
    res.set_content(lJson.dump(4), "application/json");
}

void UserService::handleCreateUser(const Request& req, Response& res){
    try{
        // In POST calls, data comes in "body" of the request
        json lBodyJson = json::parse(req.body);
        // Now, to create user we need following params - username, email, password
        // So, first make sure all are present in the request body
        if(!lBodyJson.contains("username")||
           !lBodyJson.contains("email") ||
           !lBodyJson.contains("password")){
            throw invalid_argument("Missing one or more required fields: username, email id, password");
        }
        string lUsername = lBodyJson["username"];
        string lEmailId = lBodyJson["email"];
        string lPassword = lBodyJson["password"];

        int lUserId = mDatabaseObj->createUser(lUsername, lEmailId, lPassword);
        json lResJson = {
            {"status", "SUCCESS"},
            {"data", to_string(lUserId)}
        };
        res.status = 201; // Resource created
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const json::parse_error& e){
        json lResJson = {
            {"status", "ERROR"}, 
            {"message", "Invalid JSON Format"}
        };
        res.status = 400; // Bad Request
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const invalid_argument& e){
        json lResJson = {
            {"status", "ERROR"},
            {"message", e.what()}
        };
        res.status = 400; // Bad Request
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const exception& e){
        json lResJson = {
            {"status", "ERROR"},
            {"message", e.what()}
        };
        res.status = 500; // Internal Server Error
        res.set_content(lResJson.dump(4), "application/json");
    }
}

void UserService::handleGetUser(const Request& req, Response& res){
    // In GET requests, data comes in the "query" parameter of the Request
    // BUT NOT HERE
    try{
        int lUserId = stoi(req.matches[1]); // 1 because index start from 0: req.matches[0] = "/users/123"  (entire match)
        json lUserData = mDatabaseObj->getUser(lUserId);

        json lResJson = json::object();
        lResJson["status"] = "SUCCESS";
        lResJson["data"] = lUserData.dump();

        res.status = 200;
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const invalid_argument& e){
        json lResJson = {
            {"status", "ERROR"},
            {"message", e.what()}
        };
        res.status = 400; // Bad Request
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const runtime_error& e){
        json lResJson = {
            {"status", "ERROR"},
            {"message", e.what()}
        };
        res.status = 404; // Not Found
        res.set_content(lResJson.dump(4), "application/json");
    }
    catch(const exception& e){
        json lResJson = {
            {"status", "ERROR"},
            {"message", e.what()}
        };
        res.status = 500; // Internal Server Error
        res.set_content(lResJson.dump(4), "application/json");
    }
}