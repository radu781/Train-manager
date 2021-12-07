#pragma once

#include <utility>
#include "client.hpp"

/**
 * @brief Class that manages the read/write operations from the server.
 * Both the input and output are expected to fit the following pattern:
 * [message_length][padding][message_body].
 */
class IOManager
{
public:
    /**
     * @brief Reads data from the client
     * 
     * \param client Client that sends the data
     * \return Data read
     */
    static std::string read(Client *client);

    /**
     * @brief Send data to a client
     * 
     * \param client Client to be sent data
     * \param data Data to be sent
     */
    static void send(Client *client, const std::string &data);

private:
    /**
     * @brief Allocates a buffer that contains the message byte size, a padding
     * string and the message itself that will be sent to a client
     *
     * \param str String used to create the buffer
     * \return Pair of buffer and its length
     */
    static std::pair<char *, size_t> allocateSender(const std::string &str);

    /**
     * @brief Allocates a buffer just big enough to fit the data sent by the
     * client 
     * 
     * \param str String that holds the data necessary to allocate the buffer
     * \return Pair of data read so far and the total length of the data. If
     * this length + the length of padding is greater than BUFF_SIZE, another
     * read is necessary
     */
    static std::pair<char *, size_t> allocateReader(const char *str);

    /**
     * @brief Split the input as to extract the length of the string and the
     * remaining data. This function throws if no padding ("-=-") is found.
     * 
     * \param str String to be split
     * \return Pair of remaining data and length of data (to be later used to
     * allocate a buffer just big enough to fit the data from the client) 
     */
    static std::pair<const char *, size_t> split(const char *str);

    static constexpr size_t BUFF_SIZE = 15;
    static constexpr const char *PADDING = "-=-";
};
