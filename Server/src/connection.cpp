#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <thread>
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

void Connection::closeConnection(int sock)
{
    close(sock);
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

    std::thread reader(readIndividual, sock);
    std::thread sender(sendIndividual, sock, "hello");
    reader.join();
    sender.join();
}

void Connection::run()
{
    for (auto &t : threads)
        t = std::thread(runIndividual);

    for (auto &t : threads)
        t.join();

    std::cout << "joined all\n";
}

void Connection::sendIndividual(int sock, const std::string &str)
{
    for (;;)
    {
        ssize_t sender = ::send(sock, str.c_str(), strlen(str.c_str()), 0);
        if ((size_t )sender == 0 || (size_t )sender == -1)
        {
            std::cout << "Lost connection to " << sock << '\n';
            closeConnection(sock);
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if ((size_t)sender != str.size())
        {
            std::cout << (size_t)sender << " " << strlen(str.c_str()) << '\n';
            throw std::runtime_error("Incorrect size sent");
        }

        printf("Server: %s\n", str.c_str());
    }
}

void Connection::readIndividual(int sock)
{
    for (;;)
    {
        char buff[BUFF_SIZE]{};
        ssize_t reader = ::read(sock, buff, BUFF_SIZE);
        if ((size_t)reader == 0 || strlen(buff) == 0)
        {
            std::cout << "Lost connection to " << sock << '\n';
            closeConnection(sock);
            return;
        }
        if ((size_t)reader != strlen(buff))
        {
            std::cout << (size_t)reader << " " << strlen(buff) << '\n';
            throw std::runtime_error("Incorrect size read");
        }

        std::cout << "From " << sock << ": " << buff << '\n';
    }
}
