#include "pc.h"
#include "late.hpp"

Late::Late(const Command *other, const std::vector<std::string> *command)
    : Command(other, command)
{
}

std::string Late::execute()
{
    std::string trainName;
    for (size_t i = 1; i < command->size() - 1; i++)
        trainName += command->at(1) + " ";
    trainName = WordOperation::removeDiacritics(trainName);

    auto nameFind = match(trainName, FindBy::CITY);
    if (nameFind.size() == 1)
        return "Found train name: " + trainName;
    if (nameFind.size() > 1)
    {
        std::string out;
        for (const auto &name : nameFind)
            out += name.substr(0, name.size() - 1) + ", ";
        out = out.substr(0, out.size() - 2);
        return "Found multiple matches, please refine your search:\n" + out;
    }

    auto trainFind = match(trainName, FindBy::TRAIN);
    if (trainFind.size() == 1)
        return "Found train number: " + command->at(1);
    if (trainFind.size() > 1)
    {
        std::string out;
        for (const auto &train : trainFind)
            out += train + ", ";
        out = out.substr(0, out.size() - 2);
        return "Found multiple matches, please refine your search:\n" + out;
    }
    return "Found no train name or number";
}

std::string Late::undo()
{
    return "late undo";
}
