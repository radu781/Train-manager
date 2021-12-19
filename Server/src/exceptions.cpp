#include "utils/exceptions.hpp"

ConnectionException::ConnectionException(const std::string &exception)
    : exception(exception) {}

const char *ConnectionException::what() const throw()
{
    return exception.c_str();
}

ServerSetupException::ServerSetupException(const std::string &exception)
    : exception(exception) {}

const char *ServerSetupException::what() const throw()
{
    return exception.c_str();
}
