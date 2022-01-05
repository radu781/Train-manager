#pragma once

#include "pc.h"
#include <netinet/in.h>
#include "client.hpp"

/**
 * @brief Singleton class that manages the server to clients connections
 */
class Connection
{
public:
    /**
     * @brief Get the instance object
     * 
     * \return The instance object
     */
    static Connection *getInstance();

    /**
     * @brief Main infinite loop, calls acceptIndividual() which creates a new
     * connection when a new client connects. The <socket, client> pair is
     * added to an unordered map and then runIndividual() is called on the pair
     */
    static void run();

    static int socketFD;
    // might not be safe to have this static, but it worked so far
    static sockaddr_in address;
    friend class IOManager;
private:
    /**
     * @brief Client main function that calls readIndividual() and
     * sendIndividual() in separate threads, while the main thread cleans up
     * the disconnected clients
     * 
     * \param client Client whose thread is started
     */
    static void runIndividual(Client *client);

    /**
     * @brief Establishes the connection with a client
     * 
     * \return Socket of the client that connected
     */
    static int acceptIndividual();

    /**
     * @brief Sends data to the given client. If unable to send (due to the
     * client's disconnection), the client's socket will be freed such that no
     * memory/threads are wasted
     * 
     * \param client Client to send data to 
     * \param str Data to be sent
     */
    static void sendIndividual(Client *client, const std::string &str);

    /**
     * @brief Reads data from the given client. If unable to read (due to the
     * client's disconnection), the client's socket will the freed such that no
     * memory/threads are wasted
     * 
     * \param client Client to listen to for input 
     */
    static void readIndividual(Client *client);

    Connection();

    /**
     * @brief Is called only once and sets up the connection by calling 
     * socket, setsockopt, bind and listen
     */
    static void makeConnection();

    static void makeThreads();

    /**
     * @brief Closes the connection with the client and sets a flag needed for
     * the main runIndividual() thread to clear the disconnected clients
     * 
     * \param client The client whose connection will be closed
     */
    static void closeConnection(Client *client);

    /**
     * @brief The (only) instance object
     */
    static Connection *instance;

    /**
     * @brief Hash table that stores all clients, indexed by their socket
     * value
     */
    static std::unordered_map<int, Client *> clients;

    /**
     * @brief Arbitrarily chosen port (same as on the client) to run the
     * server on
     */
    static const unsigned PORT = 8080;
    static std::mutex m;
    static const size_t prethreadCount = 100, fdOffset = 4;
};
