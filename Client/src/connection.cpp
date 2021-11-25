#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <unistd.h>
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"

Connection *Connection::instance = nullptr;
int Connection::serverFD = 0;

Connection *Connection::getInstance()
{
    if (!instance)
        instance = new Connection;
    return instance;
}

void Connection::run()
{
    makeConnection();

    std::thread reader(read);
    std::thread sender(send, "");
    reader.join();
    sender.join();
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
    static const unsigned BUFF_SIZE = 1024;

    for (;;)
    {
        char buff[BUFF_SIZE]{};
        std::cin.getline(buff, BUFF_SIZE);
        std::cin.clear();
        fflush(stdin);

        IOManager::send(serverFD, buff);
    }
}

void Connection::read()
{
    for (;;)
    {
        std::string str = IOManager::read(serverFD);
        std::cout << str << '\n';
    }
}
