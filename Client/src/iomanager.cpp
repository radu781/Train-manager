#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>
#include "../include/iomanager.hpp"
#include "../include/connection.hpp"
#include "../include/exceptions.hpp"

std::string IOManager::read(int fd)
{
    static const unsigned BUFF_SIZE = 1024;
    char buff[BUFF_SIZE]{};

    ssize_t reader = ::read(fd, buff, BUFF_SIZE);
    if (reader == 0 || reader == -1)
    {
        Connection::closeConnection();
        throw ConnectionException("Lost connection to the server");
    }
    if ((size_t)reader != strlen(buff))
    {
        std::cout << (size_t)reader << " " << strlen(buff) << '\n';
        throw std::runtime_error("Incorrect size read");
    }

    return buff;
}

void IOManager::send(int fd, const std::string &data)
{
    auto[sendMe, sendMeSize] = allocateSender(data);
    ssize_t sender = ::send(fd, sendMe, sendMeSize, 0);
    delete[] sendMe;

    if (sender == -1)
    {
        throw ConnectionException("Lost connection to the server");
        Connection::closeConnection();
    }
    if ((size_t)sender != sendMeSize)
    {
        std::cout << (size_t)sender << " " << sendMeSize << '\n';
        throw std::runtime_error("Incorrect size sent");
    }

    std::cout << "Client: " << data << '\n';
}

std::pair<char *, size_t> IOManager::allocateSender(const std::string &str)
{
    const size_t bytes = str.size() * sizeof(str[0]) + 1;
    const size_t dataSize = bytes + strlen(PADDING) + 12;
    char *allocated = new char[dataSize]{};
    snprintf(allocated, dataSize, "%lu%s%s", bytes, PADDING, str.c_str());

    return {allocated, dataSize};
}

std::pair<char *, size_t> IOManager::allocateReader(const char* buff)
{
    auto [msg, size] = split(buff);
    char *wholeMessage = new char[size]{};
    strcpy(wholeMessage, msg);

    return {wholeMessage, size};
}

std::pair<const char *, size_t> IOManager::split(const char *str)
{
    const char *paddingStart = strstr(str, PADDING);
    std::cout << str << '\n';
    if (paddingStart == nullptr)
        throw std::runtime_error("Padding not found");

    char length[BUFF_SIZE + 1]{};
    for (int i = 0; i < paddingStart - str; i++)
        length[i] = str[i];

    return {paddingStart + strlen(PADDING), atoi(length)};
}
