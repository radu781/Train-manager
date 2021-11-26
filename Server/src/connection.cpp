#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"
#include "../include/logger.hpp"

Connection *Connection::instance = nullptr;
int Connection::socketFD = 0;
sockaddr_in Connection::address{};
std::unordered_map<int, Client *> Connection::clients;

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

void Connection::run()
{
    std::thread setup([]()
                      {
                          for (;;)
                          {

                              int sock = acceptIndividual();
                              Client *client = new Client(sock);
                              clients.insert({sock, client});
                              clients[sock]->thread = std::thread(runIndividual, client);
                          }
                      });
    setup.join();

    LOG_DEBUG("all joined");
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

void Connection::closeConnection(Client *client)
{
    std::mutex m;
    m.lock();
    close(client->sock);
    client->isConnected = false;
    LOG_COMMUNICATION("Lost connection to", false, 3);
    m.unlock();
}

void Connection::runIndividual(Client *client)
{
    std::thread reader(readIndividual, client);
    std::thread sender(sendIndividual, client, "hello");
    std::cout << "Isconnected: " << client->isConnected << "\n";
    reader.join();
    sender.join();
    if (!client->isConnected)
    {
        clients.erase(client->sock);
        LOG_DEBUG("Client erased");
    }
}

int Connection::acceptIndividual()
{
    int address_length = sizeof(address), sock;
    if ((sock = ::accept(socketFD, (sockaddr *)&address, (socklen_t *)&address_length)) < 0)
        throw ConnectionException("Could not accept");

    return sock;
}

void Connection::sendIndividual(Client *client, const std::string &str)
{
    while (client->isConnected)
    {
        IOManager::send(client, str);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Connection::readIndividual(Client *client)
{
    while (client->isConnected)
    {
        std::string fromClient = IOManager::read(client);
    }
}
