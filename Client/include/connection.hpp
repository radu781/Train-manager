#pragma once

#include <string>

class Connection
{
public:
    static Connection *getInstance();
    static void run();

    static int serverFD;

private:
    Connection() {}

    static void makeConnection();
    static void closeConnection();

    static void send(const std::string &str);
    static void read();

    static Connection *instance;

    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;
};
