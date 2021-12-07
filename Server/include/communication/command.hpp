#pragma once

#include <string>
#include <vector>
#include <utility>
#include <xml/pugixml.hpp>

class Command
{
public:
    /**
     * @brief Constructs a new Command object, trims it and appends the
     * arguments to the command vector
     *
     * \param str String input from user (exactly as sent)
     */
    Command(const std::string &str);

    /**
     * @brief Actually execute the command
     *
     * \return Execution output, both if the command if valid or not
     */
    std::string execute();

    /**
     * @brief Get a message that is only sent to clients when they connect to
     * the server
     *
     * \return Message of the day
     */
    static std::string motd();
    friend class Connection;

private:
    /**
     * @brief Trims the string by removing leading and trailing whitespace
     *
     * \param str Deep copy of string to be trimmed
     * \return Trimmed string
     */
    std::string trim(std::string str);

    /**
     * @brief Check if the command and arguments inputted by the client are valid
     *
     * \return Pair of <command index (if found, otherwise -1), bool (true if
     * the argument count matches that of the command, false otherwise>
     */
    std::pair<size_t, uint8_t> validate();

    /**
     * @brief Get all trains from [start] to [destination], where
     * start == command[1] and end == command[2] after being parsed by
     * execute()
     *
     * \return String containing the start and destination times of the trains
     */
    std::string today();
    

    /**
     * @brief Command used to print information about all available commands
     *
     * \return All available commands including fixed and optional arguments
     * and their description
     */
    static std::string help();

    /**
     * @brief Load the local .xml file or download it if not found locally
     */
    static void getFile();

    /**
     * @brief Convert string with diacritics to an ascii friendly one and make
     * the first character uppercase, leaving the other lowercase
     * 
     * \param str String to be converted
     * \return Converted string
     */
    std::string normalize(std::string str);
    /**
     * @brief Change the format of time from seconds to a more human readable
     * one
     *
     * \param seconds Time in seconds as found in the .xml file used (the
     * hour:minutes:seconds format is compacted into seconds)
     * \return String that follows the format [hours]h[minutes]m[seconds]s
     */
    static std::string getTime(int seconds);

    /**
     * @brief Vector containing the user command main call and arguments
     */
    std::vector<std::string> command;

    static pugi::xml_document doc;

    enum class CommandTypes
    {
        TODAY,
        DEPARTURES,
        ARRIVALS,
        HELP,
        COUNT
    };
    struct Args
    {
        std::string command;
        uint8_t argCount;
    };
    enum class Errors
    {
        WRONG_ARG_COUNT = -2,
        TOO_MANY_ARGS
    };
    const uint8_t OPTIONAL_OFFSET = 100;
    const Args commands[(size_t)CommandTypes::COUNT] = {{"today", 2},
                                                        {"departures", 1},
                                                        {"arrivals", 1},
                                                        {"help", OPTIONAL_OFFSET + 1}};
};