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
    static const unsigned BUFF_SIZE = 15;
    char buff[BUFF_SIZE]{};

    ssize_t reader = ::read(fd, buff, BUFF_SIZE);
    if (strcmp(buff, "") == 0)
        return "";
    if (reader == 0 || reader == -1)
    {
        Connection::closeConnection();
        throw ConnectionException("Lost connection to the server");
    }
    auto [wholeMessage, size] = allocateReader(buff);

    if (size < BUFF_SIZE - strlen(PADDING))
    {
        std::string out = wholeMessage;
        delete[] wholeMessage;
        return out;
    }

    reader = ::read(fd, wholeMessage + strlen(wholeMessage), size - strlen(wholeMessage));
    if ((size_t)reader == 0 || (strlen(buff) == 0 && buff[BUFF_SIZE - 1] != 0))
    {
        Connection::closeConnection();
        return "";
    }

    std::string out = wholeMessage;
    return out;
}

void IOManager::send(int fd, const std::string &data)
{
    if (data == "")
        return;

    auto [sendMe, sendMeSize] = allocateSender(data);
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
}

std::pair<char *, size_t> IOManager::allocateSender(const std::string &str)
{
    const size_t bytes = str.size() * sizeof(str[0]) + 1;
    const size_t dataSize = bytes + strlen(PADDING) + 12;
    char *allocated = new char[dataSize]{};
    snprintf(allocated, dataSize, "%lu%s%s", bytes, PADDING, str.c_str());

    return {allocated, dataSize};
}

std::pair<char *, size_t> IOManager::allocateReader(const char *buff)
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
