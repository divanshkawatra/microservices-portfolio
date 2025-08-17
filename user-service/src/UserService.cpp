#include <iostream>
#include <nlohmann/json.hpp>
#include <functional>
#include "UserService.h"
#include "Logger.h"
#include "PasswordService.h"

using namespace std;
using json = nlohmann::json;

// **This function tells nlohmann::json how to convert our User struct into a JSON object.
void to_json(json& pJson, const User& pUser){
    pJson = json{
        {"userid", pUser.id},
        {"username", pUser.username},
        {"email", pUser.email},
        {"created_at", pUser.created_at}
    };
}

UserService::UserService(const string& pDBPath, string& pLogPath){
    mDatabaseObj = make_unique<Database>(pDBPath);
    mLogger = FileLogger::getInstance(pLogPath);
    mPasswordService = make_unique<PasswordService>();
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

    pServer.Post("/login", [this](const Request& req, Response& res){
        this->handleUserLogin(req, res);
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
        string lHashedPassword = mPasswordService->hashPassword(lPassword);

        int lUserId = mDatabaseObj->createUser(lUsername, lEmailId, lHashedPassword);
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

void UserService::handleUserLogin(const Request& req, Response& res){
    try{
        json lBodyJson = json::parse(req.body);

        if(!lBodyJson.contains("email") || !lBodyJson.contains("password")){
            throw invalid_argument("EmailId and Password are mandatory.");
        }

        string lPassword = lBodyJson["password"];
        string lEmailId = lBodyJson["email"];
        // First find the user details with given emailId or username
        optional<UserCredentials> lUserCredentials = mDatabaseObj->getUserCredentialsByEmail(lEmailId);

        json lRes = json::object();
        if(!lUserCredentials.has_value()){
            lRes["status"] = "ERROR";
            // Security Note: Don't say "User not found." A generic message is better
            // to prevent attackers from guessing valid emails.
            lRes["message"] = "Invalid EmailID or Password";
            // NOT 404: Not Found as: A login attempt is an authentication concern. Responding
            // with 401 for any authentication failure is the standard and correct approach.
            res.status = 401; // Unauthorized
            res.set_content(lRes.dump(), "application/json");
            return;
        }

        int lUserId = lUserCredentials->id;
        string lHashedPassword = lUserCredentials->password; // OR lUserCredentials.value().password;

        bool lPasswordMatches = mPasswordService->verifyPassword(lPassword, lHashedPassword);
        if(!lPasswordMatches){
            lRes["status"] = "ERROR";
            // Use the exact same generic message for both failures. This prevents "timing attacks"
            // where an attacker could learn which emails are valid based on slightly different response
            // messages or times.
            lRes["message"] = "Invalid EmailID or Password";
            res.status = 401; // Unauthorized
        }
        else{
            lRes["status"] = "SUCCESS";
            lRes["message"] = "Login successfull";
            lRes["userid"] = lUserId;
            res.status = 200; // Success

        }
        res.set_content(lRes.dump(), "application/json");

    }
    catch(exception& e){
        cout<<"Error in handleUserLogin(): "<<e.what()<<endl;
    }
}

void UserService::logMessage(const Request& req, const Response& res){
    string lLogMessage = req.method + " " + req.path + " - " + to_string(res.status);
    mLogger->log(lLogMessage, LOG_LEVEL::INFO);
}


