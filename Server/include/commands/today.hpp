#pragma once

#include "pc.h"
#include "command.hpp"

class Today
    : public Command
{
public:
    std::string execute() override;
    std::string undo() override;

private:
    /**
     * @brief Split the command member into two strings by following the rule:
     * [begin, i], (i, end], where 0 < i < command.size()
     *
     * \return 2 valid city names if found, empty strings otherwise
     */
    std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> splitNames();
};
