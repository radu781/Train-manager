#pragma once

#include "pc.h"
#include "command.hpp"

class Departures
    : public Command
{
public:
    Departures(const Command *other, const std::vector<std::string> *command);

    std::string execute() override;
    std::string undo() override;
};
