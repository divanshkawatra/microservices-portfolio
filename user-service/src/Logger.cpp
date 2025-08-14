#include <iostream>
#include <stdexcept>
#include <ctime>
#include "Logger.h"

using namespace std;

// initialize static members
shared_ptr<FileLogger> FileLogger::mInstance = nullptr;
mutex FileLogger::sObjMutex;;

ILogger::~ILogger(){

}

string ILogger::LogLevelToString(LOG_LEVEL pLogLevel){
    switch(pLogLevel){
        case ERROR: return "ERROR";
        case WARNING: return "WARNING";
        case INFO: return "INFO";
        case DEBUG: return "DEBUG";
        default: return "INFO";
    }
    return "";
}

// Constructor
FileLogger::FileLogger(string& pLogFilePath){
    mLogFile.open(pLogFilePath.c_str());
    if(!mLogFile.is_open()){
        throw runtime_error("Error while opening log file.");
    }
    mLogFile<<"Logging started."<<endl;
    mLogLevel = LOG_LEVEL::ERROR;
}

// Destructor
FileLogger::~FileLogger(){
    if(mLogFile.is_open())
        mLogFile.close();
}

shared_ptr<FileLogger> FileLogger::getInstance(string& pLogFilePath){
    // Lazy Initialization
    if(FileLogger::mInstance == nullptr){
        // FileLogger::mInstance =  make_shared<FileLogger>(pLogFilePath); // ***won't work because make_shared is a global function
                                                                            // it can't access private method of a class
        // thread-safe initialization
        lock_guard<mutex> lock(FileLogger::sObjMutex);
        if(FileLogger::mInstance == nullptr){
            FileLogger::mInstance =  shared_ptr<FileLogger>(new FileLogger(pLogFilePath));
        }
    }
    return FileLogger::mInstance;
}

void FileLogger::setLogLevel(LOG_LEVEL pLogLevel){
    mLogLevel = pLogLevel;
}

// log() to log message
void FileLogger::log(const string& pLogMsg, LOG_LEVEL pLogLevel){
    if(pLogLevel <= mLogLevel){
        // Get the current calendar time as a time_t object
        time_t now = time(0); 
    
        // Convert to a human-readable string using ctime()
        string date_time = string(ctime(&now));
        date_time.pop_back(); // remove new-line character(\n) inserted by ctime()
    
        lock_guard<mutex> lock(mFileMtx); // for thread safety
        string pLog = " [" + string(date_time) + "] [" + LogLevelToString(pLogLevel) + "]: " + pLogMsg;
        // mLogFile<<string(date_time)<<" ["<<LogLevelToString(pLogLevel)<<"]: "<<pLogMsg<<endl;
        mLogFile << pLog.c_str() << endl;
    }
}