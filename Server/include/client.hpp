#pragma once

#include <thread>

class Client
{
public:
    Client(int sock) : sock(sock) {}
    ~Client() {}

    int sock;
    int socketFD;
    std::thread thread;

    bool isConnected = true;
private:
};
