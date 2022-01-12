#include "pc.h"
#include "arrivals.hpp"

Arrivals::Arrivals(const Command *other, const std::vector<std::string> *command)
    : Command(other, command)
{
}

std::string Arrivals::execute()
{
    std::string wholeCity;
    for (size_t i = 1; i < command->size() - 1; i++)
        wholeCity += command->at(i);
    wholeCity = WordOperation::removeDiacritics(wholeCity);

    std::unordered_set<std::string> matching = match(wholeCity, FindBy::CITY);
    if (matching.empty())
        return "Please enter a valid city";

    auto trenuri = doc->child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    unsigned delta = extractTime(command->back());
    if (delta == -1u)
        return "Please try not to use negative values";
    if (delta == -2u)
        return "Please use some digits too";
    if (delta == -3u)
        return "Please use only number or literals as shown in \"help departures\"";
    if ((unsigned)delta > Time::SECONDS_IN_DAY)
        return "Please use the \"today\" command instead for timetables further than a day";

    std::vector<Train> unsorted;

    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();

        pugi::xml_node startNode;
        Stations stations;
        bool found = false;
        for (const auto &element : trasa)
        {
            unsigned timeStamp = element.attribute(OraP).as_uint();
            std::string start = WordOperation::removeDiacritics(element.attribute(staOrig).as_string());
            std::string dest = WordOperation::removeDiacritics(element.attribute(staDest).as_string());

            stations.push_back(element);
            if (matching.contains(dest))
            {
                if (Time::isBetween(Time::current(), timeStamp, Time::current() + delta) ||
                    (Time::current() + delta >= Time::SECONDS_IN_DAY &&
                     Time::isBetween(0, timeStamp, Time::current() + delta - Time::SECONDS_IN_DAY)))
                {
                    found = true;
                    break;
                }
            }
        }
        if (found)
            unsorted.push_back({tren, stations});
    }

    if (unsorted.empty())
        return "Could not find any " + std::string("departures") +
               " in the upcoming " + Types::toString<int>(delta / 60) + " minutes";
    sort(unsorted, Criteria::ARRIVAL);
    return getVerbose(unsorted) + "\n" + getBrief(unsorted, false);
}

std::string Arrivals::undo()
{
    return cannotUndo;
}
