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

/**
 * @brief Exception thrown when the server starts up
 */
class ServerSetupException
    : public std::exception
{
public:
    ServerSetupException(const std::string &exception);
    const char *what() const throw();

private:
    std::string exception;
};

class SearchNotRefined
    : public std::exception
{
public:
    SearchNotRefined(const std::string &exception);
    const char *what() const throw();

private:
    std::string exception;
};
