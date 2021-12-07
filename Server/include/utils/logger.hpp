/**
 * Header that provides two macros: LOG_COMMUNICATION and LOG_DEBUG that call
 * the Log class's communication() and debug() methods in order to log messages
 * depending on their type.
 */

#pragma once

#include <fstream>
#include <mutex>
#include <string>

#define LOG_COMMUNICATION(str, bool, int) Log::getInstance()->communication( \
    str, bool, int)
#define LOG_DEBUG(str) Log::getInstance()->debug(str, __FUNCTION__, __FILE__, \
                                                 __LINE__)

/**
 * @brief Singleton class that logs communication or debug information. Even
 * though the class methods are public, directly calling them (without using
 * the macros mentioned above is unadvised)
 */
class Log
{
public:
    /**
     * @brief Get the instance object
     *
     * \return The instance object
     */
    static Log *getInstance();

    /**
     * @brief Log information for communication purposes (server to client)
     *
     * \param str Data to be logged
     * \param sending True if data is sent, false if data is received
     * \param who File descriptor of who receives or is sent data
     */
    static void communication(const std::string &str, bool sending = false,
                              int who = 3);

    /**
     * @brief Log information for debugging purposes (raw messages, clients
     * connecting or disconnecting, etc.)
     *
     * \param str Data to be logged
     * \param function Function name where this is called
     * \param file File name where this is called
     * \param line Line number where this is called
     */
    static void debug(const std::string &str, const char *function = nullptr,
                      const char *file = nullptr, unsigned line = 0);

private:
    Log() {}

    /**
     * @brief The (only) instance object
     */
    static Log *instance;

    /**
     * @brief Stream used to log the messages
     */
    static std::ofstream out;

    /**
     * @brief Mutex used to lock the logging operations (file write)
     */
    static std::mutex m;
};
