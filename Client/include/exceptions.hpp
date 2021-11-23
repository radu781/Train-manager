#pragma once

#include <stdexcept>
#include <string>

/**
 * @brief Exception thrown upon failure to establish a connection
 */
class ConnectionException
    : public std::exception
{
public:
    ConnectionException(const std::string &exception);
    const char *what() const throw();

private:
    std::string exception;
};
