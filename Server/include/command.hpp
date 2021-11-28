#pragma once

#include <string>
#include <vector>
#include <utility>

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
     * @brief Vector containing the user command main call and arguments
     */
    std::vector<std::string> command;

    enum class CommandTypes
    {
        LOGIN,
        REGISTER,
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
    const Args commands[(size_t)CommandTypes::COUNT] = {{"login", 1},
                                                        {"register", 1},
                                                        {"help", OPTIONAL_OFFSET + 1}};
};
