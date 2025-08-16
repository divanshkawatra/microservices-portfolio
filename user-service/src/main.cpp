#include <iostream>
#include <httplib.h>
#include <string>
#include <memory>
#include <ctime>
#include <csignal>    // for signal handling
#include <filesystem> // C++17 feature - to deal with directories
#include "UserService.h"
#include "Logger.h"

using namespace std;
using namespace httplib;

using json = nlohmann::json;

// Global variable
unique_ptr<Server> gServer; // global server object so that its accessbile for signal handler

void signalHandler(int pSigNum){
    switch(pSigNum){
        case SIGINT: {
            cout<<"Interrupt Singal("<<pSigNum<<") received, Shutting down server gracefully..."<<endl;
            if(gServer) gServer->stop();
        }
        break;
        default: {
            cout<<"Singal received: "<<pSigNum<<"), no handling added for it..."<<endl;
        }
    }
}


void createDirectoryStructure(string& pPath){
    filesystem::path lFilePath = pPath;
    filesystem::path lParentDirectoryPath = lFilePath.parent_path(); // Extract parent directory path
    // create parent directories if they don't exist
    try{
        if(!filesystem::exists(lParentDirectoryPath)){
            filesystem::create_directories(lParentDirectoryPath); // create all necessary parent directories
        }
    }
    catch(exception& e){
        throw runtime_error("Error while creating directory path:" + pPath);
    }
}

int main(int argc, char* argv[]){
    try{
        if(argc < 2){
            throw invalid_argument("Usage: ./user_service <db_path> [loglevel] [port]");
        }
        string lDBPath(argv[1]); // first argument is always program's name
        
        // Initialize the global server object
        gServer = make_unique<Server>();
        if(!gServer){
            throw runtime_error("Error while creating server instance.");
        }

        // Register Signal Handler for SIGINT
        signal(SIGINT, signalHandler);

        // generate new file for each run and with unique log file name
        time_t lCurrTime = time(0);
        string lLogPath = "./logs/user_service_" + to_string(lCurrTime) + ".txt";

        // Make sure both paths - DBPath and LogsPath - exist
        createDirectoryStructure(lDBPath);
        createDirectoryStructure(lLogPath);

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

        const char* lDockerEnv = getenv("DOCKER_ENV");
        string lIPAddress = "localhost"; // OR 127.0.0.1 - listen to requests coming from this very machine
        if(lDockerEnv && lDockerEnv == string("TRUE")) lIPAddress = "0.0.0.0"; // special IP address for "listen on all interfaces"

        lUserService->setupRoutes(*gServer); // Pass the dereferenced global server
        cout<<"User Service started on http://"<<lIPAddress<<":"<<lPort<<", press Ctrl+C to stop..."<<endl;
        gServer->listen(lIPAddress, lPort); 
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