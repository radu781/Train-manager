#pragma once

#include <netinet/in.h>
#include <array>
#include <string>
#include <thread>

class Connection
{
public:
    static Connection *getInstance();
    static void run();

    static int socketFD;
    // might not be safe to have this static
    static sockaddr_in address;

private:
    static int acceptIndividual();
    static void runIndividual();

    static void sendIndividual(int sock, const std::string &str);
    static std::string readIndividual(int sock);
    struct Client
    {
        int sock;
        int socketFD;
    };

    Connection();
    static void makeConnection();
    static Connection *instance;

    static const size_t COUNT = 3;
    std::array<Client, COUNT> clients;
    static std::array<std::thread, COUNT> threads;
    
    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;
};
