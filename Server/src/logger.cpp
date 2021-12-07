#include <cassert>
#include <fstream>
#include <mutex>
#include <utils/logger.hpp>

Log *Log::instance = nullptr;
std::ofstream Log::out;
std::mutex Log::m;

Log *Log::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Log;
        remove("log.txt");
        out = std::ofstream("log.txt", std::ios_base::app);
    }
    return instance;
}

void Log::communication(const std::string &str, bool sending, int who)
{
#ifdef ENABLE_COMM_LOGGING
    std::lock_guard<std::mutex> lock(m);
    if (str == "")
        return;
    if (!sending)
    {
        out << "From " << who << ": \"" << str << "\"" << std::endl;
        return;
    }

    assert(who >= 3);
    if (who == 3)
        out << "Internal message: \"" << str << "\"" << std::endl;
    else
        out << "To " << who << ": \"" << str << "\"" << std::endl;
#endif
}

void Log::debug(const std::string &str, const char *function, const char *file, unsigned line)
{
#ifdef ENABLE_DEBG_LOGGING
    std::lock_guard<std::mutex> lock(m);
    if (function && file && line)
        out << file << ":" << line << ": debug: " << str << std::endl;
    else
        out << "Debug info only: \"" << str << "\"" << std::endl;
#endif
}
