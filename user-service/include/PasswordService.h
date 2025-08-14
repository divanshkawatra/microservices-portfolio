#ifndef PASSWORD_SERVICE_H
#define PASSWORD_SERVICE_H

#include <string>

class PasswordService{
    public:
        std::string hashPassword(std::string& pPassword);
        bool verifyPassword(const std::string& pPassword, const std::string& pHashedPassword);
};

#endif