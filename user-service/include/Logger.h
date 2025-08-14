#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <fstream> // for file handling
#include <memory>  // shared_ptr
#include <mutex>

enum LOG_LEVEL {
    ERROR,   // 0
    WARNING, // 1
    INFO,    // 2
    DEBUG    // 3
};

class ILogger{
    public:
        virtual ~ILogger(); // virtual destructor - must for virtual inheritance
        virtual void log(const std::string& pLogMessage, LOG_LEVEL pLogLevel) = 0;
        std::string LogLevelToString(LOG_LEVEL pLogLevel);
};

// Singleton FileLogger
class FileLogger: public ILogger{
    std::mutex mFileMtx;
    std::ofstream mLogFile;
    LOG_LEVEL mLogLevel;

    static std::shared_ptr<FileLogger> mInstance; // private - global point of access

    FileLogger(std::string& pLogFilePath);
    public:
        ~FileLogger();
        static std::shared_ptr<FileLogger> getInstance(std::string& pLogFilePath);
        void log(const std::string& pLogMessage, LOG_LEVEL pLogLevel) override;
        void setLogLevel(LOG_LEVEL pLogLevel);

        static std::mutex sObjMutex;
};

#endif