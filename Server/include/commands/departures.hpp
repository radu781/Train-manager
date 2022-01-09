#pragma once

#include "pc.h"
#include "command.hpp"

class Departures
    : public Command
{
public:
    std::string execute() override;
    std::string undo() override;
};
