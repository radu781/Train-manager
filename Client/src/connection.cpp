#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"

Connection *Connection::instance = nullptr;

Connection *Connection::getInstance()
{
    if (!instance)
        instance = new Connection;
    return instance;
}

void Connection::makeConnection()
{
    struct sockaddr_in serv_addr;

    if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw ConnectionException("Socket creation error");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        throw ConnectionException("IPv4 address conversion error");
    if (connect(serverFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        throw ConnectionException("Server connection error");
}

void Connection::closeConnection()
{
    close(serverFD);
}

void Connection::send(const std::string &str)
{
    ssize_t sender = ::send(serverFD, str.c_str(), strlen(str.c_str()), 0);
    if ((size_t)sender != str.size())
        throw std::runtime_error("Incorrect size sent");

    printf("Client: [%s]\n", str.c_str());
}

std::string Connection::read()
{
    char buff[Connection::BUFF_SIZE]{};
    ssize_t reader = ::read(serverFD, buff, 1024);
    if ((size_t)reader != strlen(buff))
        throw std::runtime_error("Incorrect size read");
    printf("Read: %s\n", buff);

    return buff;
}