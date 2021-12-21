#pragma once

#include "pc.h"
#include <unordered_set>
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
     * the server, must be static to be used by client thread
     *
     * \return Message of the day
     */
    static std::string motd();
    friend class Connection;
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
    std::string help();
    std::string arrivals();
    std::string departures();

private:
    /**
     * @brief Trims the string by removing leading and trailing whitespace
     *
     * \param str Deep copy of string to be trimmed
     * \return Trimmed string
     */
    std::string trim(std::string str);

private:
    enum class CommandTypes;

public:
    /**
     * @brief Check if the command and arguments inputted by the client are valid
     *
     * \return Command type to be interpreted by execute()
     */
    CommandTypes validate();

    /**
     * @brief Load the local .xml file or download it if not found locally, must
     * be static to be called by the Connection (singleton) constructor
     */
    static void getFile();

    /**
     * @brief Convert string with diacritics to an ascii friendly one and make
     * the first character uppercase, leaving the other lowercase
     *
     * \param str String to be converted
     * \return Converted string
     */
    // static std::string normalize(std::string str);

    std::string
    getVerbose(const std::vector<std::vector<pugi::xml_node>> &obj);
    std::string
    getBrief(const std::vector<std::vector<pugi::xml_node>> &obj);
    void sort(std::vector<std::vector<pugi::xml_node>> &obj);
    static bool isBefore(unsigned time);

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
    static std::unordered_set<std::string> orase;
    static std::vector<std::string> oraseFull;

private:
    /**
     * @brief Check if the unordered_set contains the string, also modify the
     * string if a close match was found
     *
     * \param str String to be searched for in the string. Substrings are
     * considered valid too
     * \return True if the string could be found
     */
    static bool setContains(std::string &str);

    /**
     * @brief Split the command member into two strings by following the rule:
     * [begin, i], (i, end], where 0 < i < command.size()
     *
     * \return 2 valid city names if found, empty strings otherwise
     */
    std::pair<std::string, std::string> split();

    std::string findByCity(const std::string &how);

    enum class CommandTypes
    {
        TODAY,
        DEPARTURES,
        ARRIVALS,
        LATE,
        HELP,
        COUNT,

        NOT_ENOUGH_ARGS,
        TOO_MANY_ARGS,
        NOT_FOUND
    };
    struct Args
    {
        unsigned mandatory;
        unsigned optional;
        CommandTypes type;
    };

    const std::unordered_map<std::string, Args> commands = {
        {"today", {2, -1u, CommandTypes::TODAY}},
        {"departures", {2, -1u, CommandTypes::DEPARTURES}},
        {"arrivals", {2, -1u, CommandTypes::ARRIVALS}},
        {"late", {1, -1u, CommandTypes::LATE}},
        {"help", {0, 1u, CommandTypes::HELP}},
    };

    static constexpr const char *OraS = "OraS", *OraP = "OraP";
    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *trainOk = "[o] ", *trainNOk = "[x] ";
};
