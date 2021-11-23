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
bool Connection::isConnected = false;

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
    if (!isConnected)
        closeConnection();
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

    isConnected = true;
}

void Connection::closeConnection()
{
    if (isConnected)
    {
        close(serverFD);
        isConnected = false;
    }
}

void Connection::send(const std::string &str)
{
    static const unsigned BUFF_SIZE = 1024;

    if (isConnected)
        for (;;)
        {
            char buff[BUFF_SIZE]{};
            std::cin.getline(buff, BUFF_SIZE);
            std::cin.clear();
            fflush(stdin);
            ssize_t sender = ::send(serverFD, buff, strlen(buff), 0);
            if ((size_t)sender == -1)
            {
                std::cout << "Lost conenction to server\n";
                closeConnection();
                return;
            }
            if ((size_t)sender != strlen(buff))
            {
                std::cout << (size_t)sender << " " << strlen(buff) << '\n';

                throw std::runtime_error("Incorrect size sent");
            }

            printf("Client: %s\n", buff);
        }
}

void Connection::read()
{
    static const unsigned BUFF_SIZE = 1024;

    if (isConnected)
        for (;;)
        {
            char buff[BUFF_SIZE]{};
            ssize_t reader = ::read(serverFD, buff, BUFF_SIZE);
            if ((size_t)reader == 0 || (size_t)reader == -1)
            {
                std::cout << "Lost connection to the server\n";
                closeConnection();
                return;
            }
            if ((size_t)reader != strlen(buff))
            {
                std::cout << (size_t)reader << " " << strlen(buff) << '\n';

                throw std::runtime_error("Incorrect size read");
            }
            printf("Read: %s\n", buff);
        }
}
