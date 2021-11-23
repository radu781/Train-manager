#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include "../include/iomanager.hpp"
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"

std::string IOManager::read(int fd)
{
    static const unsigned BUFF_SIZE = 1024;
    char buff[BUFF_SIZE]{};

    ssize_t reader = ::read(fd, buff, BUFF_SIZE);
    if (reader == 0 || reader == -1)
    {
        Connection::closeConnection();
        throw ConnectionException("Lost connection to the server");
    }
    if ((size_t)reader != strlen(buff))
    {
        std::cout << (size_t)reader << " " << strlen(buff) << '\n';
        throw std::runtime_error("Incorrect size read");
    }

    std::cout << "From server: " << buff << '\n';
    return buff;
}

void IOManager::send(int fd, const std::string &data)
{
    const size_t dataSize = data.size() * sizeof(data[0]);

    ssize_t sender = ::send(fd, data.c_str(), dataSize, 0);
    if (sender == -1)
    {
        std::cout << "Lost conenction to server\n";
        Connection::closeConnection();
        return;
    }
    if (sender != dataSize)
    {
        std::cout << (size_t)sender << " " << dataSize << '\n';

        throw std::runtime_error("Incorrect size sent");
    }

    std::cout << "Client: " << data << '\n';
}
