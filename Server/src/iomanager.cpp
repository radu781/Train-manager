#include "pc.h"
#include <arpa/inet.h>
#include "communication/client.hpp"
#include "communication/connection.hpp"
#include "communication/iomanager.hpp"

std::string IOManager::read(Client *client)
{
    char buff[BUFF_SIZE]{};

    ssize_t reader = ::read(client->getSock(), buff, BUFF_SIZE);
    if ((size_t)reader == 0 || (strlen(buff) == 0 && buff[BUFF_SIZE - 1] != 0))
    {
        Connection::closeConnection(client);
        return "";
    }
    if (strlen(buff) == 0)
        return "";

    auto [wholeMessage, size] = allocateReader(buff);

    if (size < BUFF_SIZE - strlen(PADDING))
    {
        std::string out = wholeMessage;
        delete[] wholeMessage;
        LOG_COMMUNICATION(out, false, client->getSock());
        return out;
    }

    reader = ::read(client->getSock(), wholeMessage + strlen(wholeMessage), size - strlen(wholeMessage));
    if ((size_t)reader == 0 || (strlen(buff) == 0 && buff[BUFF_SIZE - 1] != 0))
    {
        Connection::closeConnection(client);
        return "";
    }

    std::string out = wholeMessage;
    LOG_COMMUNICATION(out, false, client->getSock());
    delete[] wholeMessage;
    return out;
}

void IOManager::send(Client *client, const std::string &data)
{
    if (data == "")
        return;
    auto [sendMe, sendMeSize] = allocateSender(data);
    ssize_t sender = ::write(client->getSock(), sendMe, sendMeSize);
    delete[] sendMe;

    if ((size_t)sender == 0 || (size_t)sender == (size_t)-1)
    {
        LOG_COMMUNICATION("Lost connection to", false, 3);
        Connection::closeConnection(client);
        return;
    }
    if ((size_t)sender != sendMeSize)
    {
        std::cout << (size_t)sender << " " << sendMeSize << '\n';
        throw std::runtime_error("Incorrect size sent");
    }

    LOG_COMMUNICATION(data, true, client->getSock());
}

std::pair<char *, size_t> IOManager::allocateSender(const std::string &str)
{
    const size_t bytes = str.size() * sizeof(str[0]) + 1;
    const size_t dataSize = bytes + strlen(PADDING) + BUFF_SIZE;
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
    if (paddingStart == nullptr)
        throw std::runtime_error("Padding not found");

    char length[16]{};
    for (int i = 0; i < paddingStart - str; i++)
        length[i] = str[i];

    return {paddingStart + strlen(PADDING), atoi(length)};
}
