#include "pc.h"
#include <netinet/in.h>
#include "communication/command.hpp"
#include "communication/connection.hpp"
#include "communication/iomanager.hpp"

Connection *Connection::instance = nullptr;
int Connection::socketFD = 0;
sockaddr_in Connection::address{};
std::unordered_map<int, Client *> Connection::clients;
std::mutex Connection::m;

Connection *Connection::getInstance()
{
    if (!instance)
    {
        makeConnection();
        makeThreads();
        instance = new Connection;
    }
    return instance;
}

Connection::Connection()
{
    Command::getFile();
}

void Connection::run()
{
    std::thread setup([]()
                      {
                          for (;;)
                          {
                              int sock = acceptIndividual();
                              Client *client = new Client(sock);
                              IOManager::send(client, Command::motd());
                            if (clients.contains(sock))
                            {
                                clients[sock]->thread = std::thread(runIndividual, client);
                                LOG_DEBUG("Loaded existing thread " + Types::toString<int>(sock));
                            }
                            else
                            {
                                clients.insert({sock, new Client(sock)});
                                clients[sock]->thread = std::thread(runIndividual, client);
                                LOG_DEBUG("Loaded into new thread: " + Types::toString<int>(sock));
                            }
                          } });
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
        throw ConnectionException("Could not attach socket to port (bind), \
possible cause: Address already in use");

    if (listen(socketFD, 3) < 0)
        throw ConnectionException("Could not listen from the server!");
}

void Connection::closeConnection(Client *client)
{
    std::lock_guard<std::mutex> lock(m);
    close(client->sock);
    client->isConnected = false;
    LOG_COMMUNICATION("[Lost connection]", false, client->sock);
}

void Connection::makeThreads()
{
    LOG_DEBUG(Types::toString<size_t>(prethreadCount) + "Threads reinit");
    for (size_t i = 0; i < prethreadCount; i++)
        clients.insert({i + fdOffset, new Client(0)});
}

void Connection::runIndividual(Client *client)
{
    std::thread reader(readIndividual, client);
    // std::thread sender(sendIndividual, client, "hello");
    reader.join();
    // sender.join();
    if (!client->isConnected)
    {
        std::lock_guard<std::mutex> lock(m);
        clients.erase(client->sock);
        LOG_DEBUG("Client erased");
        if (clients.size() < prethreadCount)
            makeThreads();
    }
}

int Connection::acceptIndividual()
{
    int address_length = sizeof(address), sock;
    if ((sock = ::accept(socketFD, (sockaddr *)&address, (socklen_t *)&address_length)) < 0)
        throw ConnectionException("Could not accept");

    LOG_COMMUNICATION("[Client accepted]", false, sock);
    return sock;
}

void Connection::sendIndividual(Client *client, const std::string &str)
{
    if (client->isConnected)
    {
        IOManager::send(client, str);
        // std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Connection::readIndividual(Client *client)
{
    while (client->isConnected)
    {
        std::string fromClient = IOManager::read(client);
        Command cmd(fromClient);
        sendIndividual(client, cmd.execute());
    }
}
