#include "pc.h"
#include "utils/time.hpp"

Log *Log::instance = nullptr;
std::ofstream Log::out;
std::mutex Log::m;
std::string Log::currentPath;

Log *Log::getInstance()
{
    if (instance == nullptr)
        instance = new Log;
    return instance;
}

Log::Log()
{
    if (!std::filesystem::is_directory("Logs"))
        std::filesystem::create_directory("Logs");

    std::filesystem::path path("Logs/");
    for (auto &entry : std::filesystem::directory_iterator(path))
    {
        std::string name = entry.path().stem().string();
        std::string extension = entry.path().extension().string();

        if (extension == ".txt")
        {
            std::string cmd = "zip 'Logs/" + name + ".zip' 'Logs/" + name + extension + "'";
            system((cmd).c_str());
            system(std::string("rm 'Logs/" + name + extension + "'").c_str());
            break;
        }
    }

    currentPath = "Logs/" + Time::currentStrVerbose() + ".txt";
    out = std::ofstream(currentPath);
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

std::string Log::getName()
{
    return currentPath.substr(0, currentPath.size() - 4);
}

void Log::close()
{
    out.close();
}
