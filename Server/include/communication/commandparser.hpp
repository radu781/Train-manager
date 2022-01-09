#pragma once

#include "pc.h"
#include "commands/command.hpp"

class CommandParser
{
public:
    static Command *icmd;
    /**
     * @brief Constructs a new Command object, trims it and appends the
     * arguments to the command vector
     *
     * \param str String input from user (exactly as sent)
     */
    CommandParser(const std::string &str);

    /**
     * @brief Actually execute the command
     *
     * \return Execution output, both if the command if valid or not
     */
    std::string execute();

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

    /**
     * @brief Vector containing the user command main call and arguments
     */
    std::vector<std::string> command;

    static pugi::xml_document doc;
    static std::unordered_set<std::string> cityNames;
    static std::unordered_set<std::string> trainNumbers;

private:
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

    friend class Help;
    static const std::unordered_map<std::string, Args> commands;

    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";
};
