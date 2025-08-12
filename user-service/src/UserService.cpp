#include <iostream>
#include <nlohmann/json.hpp>
#include <functional>
#include "UserService.h"
#include "Logger.h"

using namespace std;
using json = nlohmann::json;

// **This function tells nlohmann::json how to convert our User struct into a JSON object.
void to_json(json& pJson, const User& pUser){
    pJson = json{
        {"id", pUser.id},
        {"username", pUser.username},
        {"email", pUser.email},
        {"created_at", pUser.created_at}
    };
}

UserService::UserService(const string& pDBPath, string& pLogPath){
    mDatabaseObj = make_unique<Database>(pDBPath);
    mLogger = FileLogger::getInstance(pLogPath);
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

    // Logging
    pServer.set_logger([this](const Request& req, const Response& res){
        this->logMessage(req, res);
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
        optional<User> lUserData = mDatabaseObj->getUserById(lUserId);

        json lResJson = json::object();
        if(lUserData.has_value()){
            lResJson["status"] = "SUCCESS";
            res.status = 200;

            json lUserJson = *lUserData;
            lResJson["data"] = lUserJson;
        }
        else{
            lResJson["status"] = "ERROR";
            lResJson["message"] = "No User data found for given id.";
            res.status = 404; // Missing Resource
        }

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

void UserService::logMessage(const Request& req, const Response& res){
    string lLogMessage = req.method + " " + req.path + " - " + to_string(res.status);
    mLogger->log(lLogMessage, LOG_LEVEL::INFO);
}


