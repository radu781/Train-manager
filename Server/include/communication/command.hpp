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
    std::string late();

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

    using Stations = std::vector<pugi::xml_node>;
    struct Train
    {
        pugi::xml_node root;
        Stations st;
    };

    std::string getVerbose(const std::vector<Train> &obj);
    std::string getBrief(const std::vector<Train> &obj, bool needDelim = true, bool reverse = false);

    void sort(std::vector<Train> &obj);

    /**
     * @brief Vector containing the user command main call and arguments
     */
    std::vector<std::string> command;

    static pugi::xml_document doc;
    static std::unordered_set<std::string> orase;
    static std::vector<std::string> oraseFull;

private:
    /**
     * @brief Check if the unordered_set contains the string
     *
     * \param str String to be searched for in the string. Substrings are
     * considered valid too
     * \return Vector of all matches
     */
    static std::vector<std::string> find(const std::string &str);
    static bool contains(const std::vector<std::string>& vec, const std::string& str);

    /**
     * @brief Split the command member into two strings by following the rule:
     * [begin, i], (i, end], where 0 < i < command.size()
     *
     * \return 2 valid city names if found, empty strings otherwise
     */
    std::pair<std::vector<std::string>, std::vector<std::string>> splitNames();

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
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";
};
