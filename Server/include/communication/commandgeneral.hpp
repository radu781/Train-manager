#pragma once

#include "pc.h"
#include <unordered_set>
#include "commands/command.hpp"

class CommandGeneral
{
public:
    static Command *icmd;
    /**
     * @brief Constructs a new Command object, trims it and appends the
     * arguments to the command vector
     *
     * \param str String input from user (exactly as sent)
     */
    CommandGeneral(const std::string &str);

    /**
     * @brief Actually execute the command
     *
     * \return Execution output, both if the command if valid or not
     */
    std::string execute();

    friend class Connection;

    std::string arrivals();
    std::string departures();
    std::string late();

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
    static std::unordered_set<std::string> cityNames;
    static std::unordered_set<std::string> trainNumbers;

private:
    enum class FindBy
    {
        CITY,
        TRAIN
    };
    /**
     * @brief Check if the unordered_set contains the string
     *
     * \param str String to be searched for in the string. Substrings are
     * considered valid too
     * \return Vector of all matches
     */
    static std::unordered_set<std::string> match(const std::string &str, FindBy criteria);

    /**
     * @brief Split the command member into two strings by following the rule:
     * [begin, i], (i, end], where 0 < i < command.size()
     *
     * \return 2 valid city names if found, empty strings otherwise
     */
    std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> splitNames();

    std::string findByCity(const std::string &how);
    unsigned extractTime(const std::string &str);

    static std::mutex m;

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
        {"late", {2, -1u, CommandTypes::LATE}},
        {"help", {0, 1u, CommandTypes::HELP}},
    };

    static constexpr const char *OraS = "OraS", *OraP = "OraP";
    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *trainOk = "[o] ", *trainNOk = "[x] ";
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";
};
