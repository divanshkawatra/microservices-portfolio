#include <iostream>
#include <httplib.h>
#include <string>
#include <memory>
#include <ctime>
#include "UserService.h"
#include "Logger.h"

using namespace std;
using namespace httplib;

using json = nlohmann::json;

int main(int argc, char* argv[]){
    try{
        if(argc < 2){
            throw invalid_argument("Usage: ./user_service <db_path> [loglevel] [port]");
        }
        string lDBPath(argv[1]); // first argument is always program's name
        time_t lCurrTime = time(0);

        string lLogPath = "../logs/user_service_" + to_string(lCurrTime) + ".txt";
        unique_ptr<UserService> lUserService = make_unique<UserService>(lDBPath, lLogPath);
        shared_ptr<FileLogger> lLogger = FileLogger::getInstance(lLogPath);

        LOG_LEVEL lLogLevel = LOG_LEVEL::ERROR;
        if(argc == 3){
            lLogLevel = (LOG_LEVEL)(stoi(argv[2]));
            lLogger->setLogLevel(lLogLevel);
        }
        // create server to start listening
        int lPort = 8001;
        if(argc == 4){
            lPort = stoi(argv[3]);
        }
        Server lServer;
        lUserService->setupRoutes(lServer);
        cout<<"User Service started on http://localhost:8001, press Ctrl+C to stop..."<<endl;
        lServer.listen("localhost", lPort);
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


// ./user_service ../data/user_db.db 2