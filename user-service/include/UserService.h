#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <httplib.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "Database.h"
#include "Logger.h"

using namespace httplib;
using json = nlohmann::json;

class UserService{
    std::unique_ptr<Database> mDatabaseObj;
    std::shared_ptr<ILogger> mLogger;

    public:
        UserService(const std::string& pDbPath, std::string& pLogPath);
        void setupRoutes(httplib::Server& pServer);

    private:
        // Functions to handle different endpoints
        void handleHealthCall(const Request& req, Response& res);
        void handleCreateUser(const Request& req, Response& res);
        void handleGetUser(const Request& req, Response& res);
        void logMessage(const Request& req, const Response& res);

};

#endif