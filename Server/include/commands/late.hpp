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
};
