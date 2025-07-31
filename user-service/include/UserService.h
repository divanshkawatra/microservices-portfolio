#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <httplib.h>
#include <memory>
#include "Database.h"

using namespace std;
using namespace httplib;

class UserService{
    unique_ptr<Database> mDatabaseObj;

    public:
        UserService(const string& pDbPath);
        void setupRoutes(httplib::Server& pServer);

    private:
        // Functions to handle different endpoints
        void handleHealthCall(const Request& req, Response& res);
        void handleCreateUser(const Request& req, Response& res);
        void handleGetUser(const Request& req, Response& res);
};

#endif