#pragma once

#include <string>

class IOManager
{
public:
    static std::string read(int fd);
    static void send(int fd, const std::string &data);
};
