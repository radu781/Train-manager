#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>

class Connection
{
public:
    static Connection *getInstance();
    int accept();

    void send(int sock, const std::string &str);
    std::string read(int sock);

    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;

    static int socketFD;
    // might not be safe to have this static
    static sockaddr_in address;
private:
    struct Client
    {
        int sock;
        int socketFD;
    };

    Connection();
    static int makeConnection();
    static Connection *instance;
    std::vector<Client> clients;
};
