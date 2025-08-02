#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <fstream> // for file handling
#include <memory>  // shared_ptr
#include <mutex>

using namespace std;

enum LOG_LEVEL {
    ERROR,   // 0
    WARNING, // 1
    INFO,    // 2
    DEBUG    // 3
};

class ILogger{
    public:
        virtual ~ILogger(); // virtual destructor - must for virtual inheritance
        virtual void log(const string& pLogMessage, LOG_LEVEL pLogLevel) = 0;
        string LogLevelToString(LOG_LEVEL pLogLevel);
};

// Singleton FileLogger
class FileLogger: public ILogger{
    mutex mFileMtx;
    ofstream mLogFile;
    LOG_LEVEL mLogLevel;
    static shared_ptr<FileLogger> mInstance; // public - global point of access

    FileLogger(string& pLogFilePath);
    public:
        ~FileLogger();
        static shared_ptr<FileLogger> getInstance(string& pLogFilePath);
        void log(const string& pLogMessage, LOG_LEVEL pLogLevel) override;
        void setLogLevel(LOG_LEVEL pLogLevel);
};

#endif