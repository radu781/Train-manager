#pragma once

#include <utility>
#include <string>

class IOManager
{
public:
    static std::string read(int fd);
    static void send(int fd, const std::string &data);

private:
    static std::pair<char *, size_t> allocateSender(const std::string &str);
};
