#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"

Connection *Connection::instance = nullptr;
int Connection::socketFD = 0;
sockaddr_in Connection::address{};
std::array<std::thread, 3> Connection::threads;

Connection *Connection::getInstance()
{
    if (!instance)
    {
        makeConnection();
        instance = new Connection;
    }
    return instance;
}

Connection::Connection()
{
}

void Connection::makeConnection()
{
    int optVal = 1;

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        throw ConnectionException("Socket creation error");
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)))
        throw ConnectionException("Could not attach socket to port (setsockopt)");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socketFD, (struct sockaddr *)&address, sizeof(address)) < 0)
        throw ConnectionException("Could not attach socket to port (bind)");

    if (listen(socketFD, 3) < 0)
        throw ConnectionException("Could not listen from the server!");
}

int Connection::acceptIndividual()
{
    int address_length = sizeof(address), sock;
    if ((sock = ::accept(socketFD, (sockaddr *)&address, (socklen_t *)&address_length)) < 0)
        throw ConnectionException("Could not accept");

    return sock;
}

void Connection::runIndividual()
{
    int sock = acceptIndividual();
    std::cout << "started " << std::this_thread::get_id() << '\n';

    readIndividual(sock);
    sendIndividual(sock, "hello");
}

void Connection::run()
{
    int index = 0;
    for (auto &t : threads)
        t = std::thread(runIndividual);

    for (auto &t : threads)
        t.join();

    std::cout << "joined all\n";
}

void Connection::sendIndividual(int sock, const std::string &str)
{
    ssize_t sender = ::send(sock, str.c_str(), strlen(str.c_str()), 0);
    if ((size_t)sender != str.size())
        throw std::runtime_error("Incorrect size sent");

    printf("Server: [%s]\n", str.c_str());
}

std::string Connection::readIndividual(int sock)
{
    char buff[BUFF_SIZE]{};
    ssize_t reader = ::read(sock, buff, BUFF_SIZE);
    if ((size_t)reader != strlen(buff))
        throw std::runtime_error("Incorrect size read");
        
    std::cout << "From " << sock << ": " << buff << '\n';
    return buff;
}
