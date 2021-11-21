#pragma once

#include <stdexcept>
#include <string>

class ConnectionException
    : public std::exception
{
public:
    ConnectionException(const std::string &exception);
    const char *what() const throw();

private:
    std::string exception;
};
