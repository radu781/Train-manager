#pragma once

#include <string>

class Connection
{
public:
    static Connection *getInstance();
    void makeConnection();

    void send(int sock, const std::string &str);
    std::string read(int sock);

    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;

private:
    struct Client
    {
        int sock;
        int socketFD;
    };

public:
    Client client;

private:
    Connection() {}
    static Connection *instance;
};
