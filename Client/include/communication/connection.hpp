#pragma once

#include <string>
#include "iomanager.hpp"

/**
 * @brief Singleton class that manages the client to server connection
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
     * @brief Main infinite loop, calls read() and send() in separate threads,
     * while the main one closes the connection if needed
     */
    static void run();

    static int serverFD;
    friend class IOManager;
private:
    Connection() {}

    /**
     * @brief Is called only once and sets up the connection by calling 
     * socket, inet_pton() and connect
     */
    static void makeConnection();

    /**
     * @brief Closes the connection with the server and sets a flag needed for
     * the main run() thread. An exception should be thrown shortly after this
     * is called.
     */
    static void closeConnection();

    /**
     * @brief Sends data to the server. If unable to send (due to the server's
     * disconnection), the client's connection will be closed
     * 
     * \param str Data to be sent
     */
    static void send(const std::string &str);

    /**
     * @brief Reads data from the given server. If unable to read (due to the
     * server's disconnection), the client's connection will be closed
     */
    static void read();

    /**
     * @brief The (only) instance object
     */
    static Connection *instance;

    /**
     * @brief Arbitrarily chosen port (same as on the server) to run the
     * server on
     */
    static const unsigned PORT = 8080;
};
