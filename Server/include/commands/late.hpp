#pragma once

#include "pc.h"
#include "command.hpp"

class Late
    : public Command
{
public:
    Late(const Command *other, const std::vector<std::string> *command);

    std::string execute() override;
    std::string undo() override;

    enum class FindResults
    {
        FOUND_STATION,
        FOUND_NUMBER,
        FOUND_MORE_STATIONS,
        FOUND_MORE_NUMBERS,
        FOUND_NONE
    };

    std::pair<FindResults, std::unordered_set<std::string>> find(const std::string &toFind);
};
