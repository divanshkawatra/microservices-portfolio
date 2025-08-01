#include <iostream>
#include <httplib.h>
#include <string>
#include <memory>
#include <ctime>
#include "UserService.h"

using namespace std;
using namespace httplib;

using json = nlohmann::json;


int main(int argc, char* argv[]){
    try{
        if(argc < 2){
            throw invalid_argument("Usage: ./user_service <db_path> [port]");
        }
        string lDBPath(argv[1]); // first argument is always program's name
        time_t lCurrTime = time(0);

        string lLogPath = "../logs/user_service_" + to_string(lCurrTime) + ".txt";
        unique_ptr<UserService> lUserService = make_unique<UserService>(lDBPath, lLogPath);

        // create server to start listening
        int port = 8001;
        if(argc == 3){
            port = stoi(argv[2]);
        }
        Server lServer;
        lUserService->setupRoutes(lServer);
        cout<<"User Service started on http://localhost:8001, press Ctrl+C to stop..."<<endl;
        lServer.listen("localhost", 8001);
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