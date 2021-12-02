#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <communication/command.hpp>

Command::Command(const std::string &str)
{
    if (str == "")
        return;

    std::string trimmed = trim(str);
    std::istringstream in(trimmed);
    std::string token;
    // TODO: add more delims in the future
    while (std::getline(in, token, ' '))
        command.push_back(token);
}

std::pair<size_t, uint8_t> Command::validate()
{
    std::transform(command[0].begin(), command[0].end(), command[0].begin(),
                   tolower);

    if (command.size() >= (uint8_t)-1)
        return {-1, 0};

    for (size_t i = 0; i < (size_t)CommandTypes::COUNT; i++)
        if (command[0] == commands[i].command)
        {
            // if command with optional values
            if (commands[i].argCount > OPTIONAL_OFFSET)
            {
                if (command.size() - 1 <= commands[i].argCount - OPTIONAL_OFFSET)
                    return {i, command.size() - 1};
                else
                    // too many args
                    return {i, (uint8_t)Errors::TOO_MANY_ARGS};
            }
            // if command with fixed number of values
            else if (commands[i].argCount < OPTIONAL_OFFSET)
            {
                if (command.size() - 1 == commands[i].argCount)
                    return {i, command.size() - 1};
                else
                    // wrong arg count
                    return {i, (uint8_t)Errors::WRONG_ARG_COUNT};
            }
        }

    return {-1, 0};
}

std::string Command::execute()
{
    if (command.size() == 0)
        return "";

    auto [cmd, argc] = validate();
    if (cmd == (size_t)-1)
        return "Command " + command[0] + " is (currently) not supported";
    if (argc == (uint8_t)Errors::TOO_MANY_ARGS)
        return "Command " + command[0] + " has " +
               (char)(commands[cmd].argCount + '0' - OPTIONAL_OFFSET) +
               " optional arguments, " + (char)(command.size() - 1 + '0') + " provided";
    if (argc == (uint8_t)Errors::WRONG_ARG_COUNT)
        return "Command " + command[0] + " has " +
               (char)(commands[cmd].argCount + '0') + " arguments, " +
               (char)(command.size() - 1 + '0') + " provided";

    return "Valid command " + command[0];
}

std::string Command::today()
{
    return "";
}

std::string Command::trim(std::string str)
{
    if (str == "")
        return "";

    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c)
                                        { return !std::isspace(c); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char c)
                           { return !std::isspace(c); })
                  .base(),
              str.end());

    return str;
}
