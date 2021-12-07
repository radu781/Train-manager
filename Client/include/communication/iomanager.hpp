#pragma once

#include <utility>
#include <string>

class IOManager
{
public:
    /**
     * @brief Reads data from the client
     * 
     * \param fd Server file descriptor
     * \return Data read
     */
    static std::string read(int fd);

    /**
     * @brief Send data to the server
     * 
     * \param fd Server file descriptor
     * \param data Data to be sent
     */
    static void send(int fd, const std::string &data);

private:
    /**
     * @brief Allocates a buffer that contains the message byte size, a padding
     * string and the message itself that will be sent to the server
     *
     * \param str String used to create the buffer
     * \return Pair of buffer and its length
     */
    static std::pair<char *, size_t> allocateSender(const std::string &str);

    /**
     * @brief Allocates a buffer just big enough to fit the data sent by the
     * server
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
