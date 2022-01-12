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

    unsigned delta = extractTime(command->back());
    std::string deltaStr = Types::toString<unsigned>(delta / 60);

    auto [res, set] = find(trainName);
    std::string out;
    switch (res)
    {
    case FindResults::FOUND_STATION:
        // can't dereference begin because it invalidates the set's ordering
        // so this is the only way
        for (const auto &it : set)
            out += it;
        return "Found train by station name: " + out + " and proposed delay of " +
               deltaStr + " minutes";

    case FindResults::FOUND_NUMBER:
        return "Found train by number: " + command->at(1) + "and proposed delay of" +
               deltaStr + " minutes";

    case FindResults::FOUND_MORE_STATIONS:
        for (const auto &name : set)
            out += name.substr(0, name.size() - 1) + ", ";
        out = out.substr(0, out.size() - 2);
        return "Found multiple matches, please refine your search:\n" + out;

    case FindResults::FOUND_MORE_NUMBERS:
        for (const auto &train : set)
            out += train + ", ";
        out = out.substr(0, out.size() - 2);
        return "Found multiple matches, please refine your search:\n" + out;

    case FindResults::FOUND_NONE:
        return "Found no station name or train number";

    default:
        LOG_DEBUG("Got unexpected enum value: " + Types::toString<unsigned>((unsigned)res));
        return "";
    }
}

std::string Late::undo()
{
    return "Undid delay of " + Types::toString<unsigned>(extractTime(command->back()) / 60) +
           " minutes";
}

std::pair<Late::FindResults, std::unordered_set<std::string>> Late::find(const std::string &toFind)
{
    auto nameFind = match(toFind, FindBy::CITY);

    if (nameFind.size() == 1)
    {
        std::unordered_set<std::string> out;
        for (const auto &it : nameFind)
            out.insert(it);
        return {FindResults::FOUND_STATION, out};
    }
    if (nameFind.size() > 1)
        return {FindResults::FOUND_MORE_STATIONS, nameFind};

    auto trainFind = match(toFind, FindBy::TRAIN);
    if (trainFind.size() == 1)
    {
        std::unordered_set<std::string> out;
        for (const auto &it : nameFind)
            out.insert(it);
        return {FindResults::FOUND_NUMBER, out};
    }
    if (trainFind.size() > 1)
        return {FindResults::FOUND_MORE_NUMBERS, trainFind};

    return {FindResults::FOUND_NONE, {}};
}
