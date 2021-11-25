#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <unistd.h>
#include "../include/client.hpp"
#include "../include/connection.hpp"
#include "../include/iomanager.hpp"

std::string IOManager::read(Client *client)
{
    char buff[BUFF_SIZE]{};

    // read BUFF_SIZE bytes from client
    ssize_t reader123 = ::read(client->sock, buff, BUFF_SIZE);
    if ((size_t)reader123 == 0 || (strlen(buff) == 0 && buff[BUFF_SIZE - 1] != 0))
    {
        std::cout << "Lost connection to {READ} " << client->sock << '\n';
        Connection::closeConnection(client);
        return "";
    }
    // if (strlen(buff123) != reader123)
    // {
    //     std::cout << (size_t)reader123 << " " << strlen(buff123) << '\n';
    //     throw std::runtime_error("Incorrect size read");
    // }

    auto [wholeMessage, size] = allocateReader(buff);

    if (size < BUFF_SIZE - strlen(PADDING))
    {
        std::string out = wholeMessage;
        delete[] wholeMessage;
        return out;
    }

    ::read(client->sock, wholeMessage + strlen(wholeMessage), size - strlen(wholeMessage));

    std::string out = wholeMessage;
    delete[] wholeMessage;
    return out;
}

void IOManager::send(Client *client, const std::string &data)
{
    ssize_t sender = ::send(client->sock, data.c_str(), data.size(), 0);
    if ((size_t)sender == 0 || (size_t)sender == (size_t)-1)
    {
        std::cout << "Lost connection to " << client->sock << '\n';
        Connection::closeConnection(client);
        return;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if ((size_t)sender != data.size())
    {
        std::cout << (size_t)sender << " " << data.size() << '\n';
        throw std::runtime_error("Incorrect size sent");
    }

    printf("Server->%d: %s\n", client->sock, data.c_str());
}

std::pair<char *, size_t> IOManager::allocateSender(const std::string &str)
{
    return {};
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

    char length[16]{};
    for (int i = 0; i < paddingStart - str; i++)
        length[i] = str[i];

    return {paddingStart + strlen(PADDING), atoi(length)};
}
