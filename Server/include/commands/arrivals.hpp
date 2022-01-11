#pragma once

#include "pc.h"
#include "command.hpp"

class Arrivals
    : public Command
{
public:
    Arrivals(const Command *other, const std::vector<std::string> *command);

    std::string execute() override;
    std::string undo() override;
};
