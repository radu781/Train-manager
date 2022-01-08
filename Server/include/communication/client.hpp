#pragma once

#include "pc.h"

/**
 * @brief Stores information regarding a single client, so far only a socket
 * for read/send operations and a main thread
 */
class Client
{
public:
    Client() = default;
    Client(sockaddr_in addr, int socketFD);
    ~Client() {}

    int accept();
    int getSock() const;
    int getSockFD() const;
    const char* getIP() const;

    std::thread thread;
    bool isConnected = true;

private:
    int sock;
    int socketFD;
    sockaddr_in address;

    static const unsigned PORT = 8080;
};
