#pragma once

#include "pc.h"

/**
 * @brief Stores information regarding a single client, so far only a socket
 * for read/send operations and a main thread 
 */
class Client
{
public:
    Client(int sock) : sock(sock) {}
    ~Client() {}

    int sock;
    int socketFD;
    std::thread thread;

    bool isConnected = true;
    std::string username;
private:
};
