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
    static std::pair<char *, size_t> allocateReader(const char *str);
    static std::pair<const char *, size_t> split(const char *str);

    static constexpr size_t BUFF_SIZE = 15;
    static constexpr const char *PADDING = "-=-";
};
