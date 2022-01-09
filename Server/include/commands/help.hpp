#pragma once

#include "pc.h"
#include "command.hpp"

class Help
    : public Command
{
public:
    std::string execute() override;
    std::string undo() override;
};
