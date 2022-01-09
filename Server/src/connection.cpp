#include "pc.h"
#include <signal.h>
#include "communication/commandparser.hpp"
#include "communication/connection.hpp"
#include "communication/iomanager.hpp"
#include "commands/motd.hpp"

Connection *Connection::instance = nullptr;
int Connection::socketFD = 0;
sockaddr_in Connection::address{};
std::unordered_map<int, Client *> Connection::clients;
std::mutex Connection::m;

Connection *Connection::getInstance()
{
    if (!instance)
        instance = new Connection;
    return instance;
}

Connection::Connection()
{
    CommandParser::getFile();
    makeThreads();
    makeConnection();
}

void Connection::run()
{
    signal(SIGINT, processSignal);
    std::thread setup(Connection::setup);
    setup.join();

    LOG_DEBUG("all joined, server closing");
}

void Connection::setup()
{
    for (;;)
    {
        Client *client = acceptIndividual(address, socketFD);
        int sock = client->getSock();
        if (clients.contains(sock))
        {
            clients[sock]->thread = std::thread(runIndividual, client);
            LOG_DEBUG("Loaded existing thread " + Types::toString<int>(sock));
        }
        else
        {
            clients.insert({sock, new Client()});
            clients[sock]->thread = std::thread(runIndividual, client);
            LOG_DEBUG("Loaded into new thread: " + Types::toString<int>(sock));
        }
    }
}

void Connection::processSignal(int sig)
{
    if (sig == SIGINT)
    {
        LOG_DEBUG("Got SIGINT, closing all connections");
        for (auto &[socket, client] : clients)
            closeConnection(client);

        exit(0);
    }
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
    close(client->getSock());
    client->isConnected = false;
    if (client->getSock()) // set to 0 when server gets SIGINT
        LOG_COMMUNICATION("[Lost connection]", false, client->getSock());

    clients.erase(client->getSock());
    delete client;
}

void Connection::makeThreads()
{
    LOG_DEBUG(Types::toString<size_t>(prethreadCount) + " Threads reinit");
    for (size_t i = 0, j = 0; i < prethreadCount; i++)
    {
        for (j = i + fdOffset; clients.contains(j); j++)
            ;
        clients.insert({j, new Client()});
    }
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
        LOG_DEBUG("Client erased");
        if (clients.size() < prethreadCount / 5)
            makeThreads();
    }
}

Client *Connection::acceptIndividual(sockaddr_in addr, int socket)
{
    Client *client = new Client(address, socket);
    if (client->accept() < 0)
        throw ConnectionException("Could not accept");

    LOG_COMMUNICATION(std::string("[Client accepted]") + client->getIP(), false, client->getSock());
    client->cmd = new Motd();
    IOManager::send(client, client->cmd->execute());
    delete client->cmd;
    return client;
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
        CommandParser cmd(fromClient);
        sendIndividual(client, cmd.execute());
    }
}
