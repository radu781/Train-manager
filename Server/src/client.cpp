#include "communication/client.hpp"

Client::Client(sockaddr_in addr, int socketFD)
    : socketFD(socketFD), address(addr)
{
}

int Client::accept()
{
    int address_length = sizeof(address);
    sock = ::accept(socketFD, (sockaddr *)&address, (socklen_t *)&address_length);
    return sock;
}
int Client::getSock() const
{
    return sock;
}

int Client::getSockFD() const
{
    return socketFD;
}

const char *Client::getIP() const
{
    return inet_ntoa((in_addr)address.sin_addr);
}
