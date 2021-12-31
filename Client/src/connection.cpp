#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <unistd.h>
#include "communication/connection.hpp"
#include "utils/exceptions.hpp"
#include "loadingbar.hpp"

Connection *Connection::instance = nullptr;
int Connection::serverFD = 0;
bool Connection::connected = false;

Connection *Connection::getInstance()
{
    if (!instance)
        instance = new Connection;
    return instance;
}

void Connection::run()
{
    try
    {
        makeConnection();
    }
    catch (const ConnectionException &e)
    {
        std::cout << "The server is unavailable now, please try again later\n";
        return;
    }

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

    const unsigned RETRY_ATTEMPTS = 5, TIME = 1;
    LoadingBar bar(RETRY_ATTEMPTS, TIME);
    for (unsigned i = 0; i < RETRY_ATTEMPTS; i++)
        if (connect(serverFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            char buf[64];
            unsigned seconds = TIME + i * 2;
            sprintf(buf, "%u/%u Connection failed, retrying in %u seconds", i + 1, RETRY_ATTEMPTS, seconds);
            std::cout << buf << std::flush;
            bar.update(seconds);
        }
        else
        {
            connected = true;
            return;
        }

    throw ConnectionException("Server connection error");
}

void Connection::closeConnection()
{
    close(serverFD);
    connected = false;
}

void Connection::send(const std::string &str)
{
    static const unsigned BUFF_SIZE = 1024;

    while (connected)
    {
        char buff[BUFF_SIZE]{};
        std::cin.getline(buff, BUFF_SIZE);
        std::cin.clear();
        fflush(stdin);
        if (strcmp(buff, "quit") == 0)
        {
            shutdown(serverFD, SHUT_RDWR);
            close(serverFD);
            break;
        }

        IOManager::send(serverFD, buff);
    }
}

void Connection::read()
{
    while (connected)
    {
        std::string str = IOManager::read(serverFD);
        if (str != "")
            std::cout << str << '\n';
    }
}
