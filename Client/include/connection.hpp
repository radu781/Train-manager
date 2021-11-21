#pragma once

#include <string>

class Connection
{
public:
    static Connection *getInstance();
    void makeConnection();
    void closeConnection();

    void send(const std::string &str);
    std::string read();

    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;

    int serverFD;
private:
    Connection() {}
    static Connection *instance;
};
