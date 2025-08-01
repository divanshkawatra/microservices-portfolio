#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

enum LOG_LEVEL {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class ILogger{
    public:
        virtual ~ILogger(); // virtual destructor - must for virtual inheritance
        virtual void log(string& pLogMessage, LOG_LEVEL pLogLevel) = 0;
        string LogLevelToString(LOG_LEVEL pLogLevel);
};

// Singleton FileLogger
class FileLogger: public ILogger{
    ofstream mLogFile;
    LOG_LEVEL mLogLevel;

    FileLogger(string& pLogFilePath);
    public:
        static shared_ptr<FileLogger> mInstance; // public - global point of access

        ~FileLogger();
        static shared_ptr<FileLogger> getInstance(string& pLogFilePath);
        void log(string& pLogMessage, LOG_LEVEL pLogLevel) override;
        void setLogLevel(LOG_LEVEL pLogLevel);
};

#endif