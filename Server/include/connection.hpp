#pragma once

#include <netinet/in.h>
#include <unordered_map>
#include <string>
#include <thread>
#include "../include/client.hpp"

class Connection
{
public:
    static Connection *getInstance();
    static void run();

    static int socketFD;
    // might not be safe to have this static
    static sockaddr_in address;

private:
    static void runIndividual(Client* client);
    static int acceptIndividual();
    static void sendIndividual(Client* client, const std::string &str);
    static void readIndividual(Client* client);

    Connection();
    static void makeConnection();
    static void closeConnection(Client* client);
    static Connection *instance;

    // Int is the client socket used for communication, 
    // Client* is the actual client
    static std::unordered_map<int, Client *> clients;

    static const unsigned PORT = 8080;
    static const unsigned BUFF_SIZE = 1024;
};
