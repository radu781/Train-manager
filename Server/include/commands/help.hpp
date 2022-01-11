#pragma once

#include "pc.h"
#include "command.hpp"

class Help
    : public Command
{
public:
    Help(const Command *other, const std::vector<std::string> *command);

    std::string execute() override;
    std::string undo() override;
};
