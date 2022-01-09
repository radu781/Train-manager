#include "pc.h"
#include "today.hpp"
#include "utils/wordoperation.hpp"

std::string Today::execute()
{
    if (command[1] == command[2])
        return "Please enter different start and destinations";

    auto trenuri = doc->child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    std::unordered_set<std::string> startVec, destVec;
    try
    {
        auto [start, dest] = splitNames();
        startVec = start;
        destVec = dest;
    }
    catch (const SearchNotRefined &e)
    {
        return "Please refine your search";
    }

    if (startVec.empty() || destVec.empty())
        return "Did not match start or destination city";

    std::vector<Train> unsorted;
    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();
        pugi::xml_node startNode, destNode;
        Stations stations;
        for (const auto &element : trasa)
        {
            std::string testStart = WordOperation::removeDiacritics(element.attribute(staOrig).as_string());
            std::string testDest = WordOperation::removeDiacritics(element.attribute(staOrig).as_string());

            if (startVec.contains(testStart))
                startNode = element;
            if (!startNode.empty())
                stations.push_back(element);

            if (destVec.contains(testDest))
            {
                destNode = element;
                if (!stations.empty())
                    stations.pop_back();
                break;
            }
        }

        if (!stations.empty() && !destNode.empty())
            unsorted.push_back({tren, stations});
    }

    Command::sort(unsorted, Criteria::DEPARTURE);
    return getVerbose(unsorted) + "\n" + getBrief(unsorted);
}

std::string Today::undo()
{
    return "";
}

std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> Today::splitNames()
{
    const unsigned MIN_LENGTH_THRESHOLD = 3;
    for (size_t i = 1; i < command->size() - 1; i++)
        if (command[i].size() < MIN_LENGTH_THRESHOLD)
            throw SearchNotRefined("");

    for (size_t i = 1; i < command->size() - 1; i++)
    {
        std::string start = command->at(1), dest = command->at(i + 1);
        for (size_t j = 2; j <= i; j++)
            start += " " + command->at(j);
        for (size_t j = i + 2; j < command->size(); j++)
            dest += " " + command->at(j);

        auto startVec = match(WordOperation::removeDiacritics(start), FindBy::CITY);
        auto endVec = match(WordOperation::removeDiacritics(dest), FindBy::CITY);
        if (!startVec.empty() && !endVec.empty())
            return {startVec, endVec};
    }
    return {};
}
