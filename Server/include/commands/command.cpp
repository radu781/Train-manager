#include "pc.h"
#include "command.hpp"

pugi::xml_document *Command::doc;
std::unordered_set<std::string> *Command::cityNames;
std::unordered_set<std::string> *Command::trainNumbers;
std::mutex Command::m;

void Command::setCommand(std::vector<std::string> *command)
{
    this->command = command;
}

void Command::init(pugi::xml_document *doc, std::unordered_set<std::string> *cities, std::unordered_set<std::string> *trains)
{
    this->doc = doc;
    this->cityNames = cities;
    this->trainNumbers = trains;
}

std::string Command::getVerbose(const std::vector<Train> &obj)
{
    std::string out;
    unsigned index = 0;

    const unsigned MIN_OFFSET = 3;
    const unsigned PRINT_THRESHOLD = 1;
    bool canPrint = false;
    for (const auto &vec : obj)
    {
        unsigned minCount = 0;
        std::string trainType = vec.root.attribute(CatTren).as_string();
        std::string trainNumber = vec.root.attribute(Numar).as_string();
        std::string tmp = Types::toString<unsigned>(++index) + ". " + trainType + trainNumber + "\n";

        for (const auto &node : vec.st)
        {
            unsigned start = node.attribute(OraP).as_int();
            unsigned end = node.attribute(OraS).as_int();

            std::string orig = node.attribute(staOrig).as_string();
            std::string dest = node.attribute(staDest).as_string();

            std::string delta =
                end - start < 3600
                    ? Time::toString(end - start).substr(MIN_OFFSET) + " min"
                    : Time::toString(end - start);

            tmp += "(" + Time::toString(start) + " -> " + Time::toString(end) + ", " + delta +
                   ") " + orig + " -> " + dest + "\n";

            minCount++;
        }
        if (minCount > PRINT_THRESHOLD)
        {
            out += tmp + "\n";
            canPrint = true;
        }
    }

    if (!index)
        return "Found no trains";
    if (!canPrint)
        return "";
    out = out.substr(0, out.size() - 1); // overhead but looks better

    return "Found " + Types::toString<unsigned>(index) + " trains:\n" + out;
}

std::string Command::getBrief(const std::vector<Train> &obj, bool needDelim, bool reverse)
{
    std::string out;
    unsigned index = 0;
    unsigned availableTrains = 0;

    for (const auto &vec : obj)
    {
        std::string available = (Time::current() < vec.st.front().attribute(OraP).as_uint()) ? trainOk : trainNOk;
        availableTrains += (available == trainOk);

        std::string trainType = vec.root.attribute(CatTren).as_string();
        std::string trainNumber = vec.root.attribute(Numar).as_string();

        unsigned start = vec.st.front().attribute(OraP).as_int();
        unsigned end = vec.st.back().attribute(OraS).as_int();

        std::string orig = vec.st.front().attribute(staOrig).as_string();
        std::string dest = vec.st.back().attribute(staDest).as_string();
        if (reverse)
            std::swap(orig, dest);

        std::string delta = Time::diffToString(end, start);
        if (availableTrains == 1 && needDelim)
            out += "----------\n";
        out += (needDelim ? available : "[o] ") + RPAD(Types::toString<unsigned>(++index) + ". ", 4) +
               RPAD(trainType + trainNumber, 11) +
               "(" + Time::toString(start) + " -> " + Time::toString(end) + ", " +
               delta + ") " + orig + " -> " + dest + "\n";
    }

    if (!index)
        return "";

    const std::string info = LPAD("Number", 10) + LPAD("Depart", 15) + LPAD("Arrival", 11) + LPAD("Time", 5) + "\n";
    if (!availableTrains)
        return "No trains available at this time\n" + info + out;
    return (needDelim
                ? "At a glance (" + Types::toString<unsigned>(availableTrains) + "/" +
                      Types::toString<unsigned>(index) + " trains available)\n"
                : "") +
           info + out;
}

void Command::sort(std::vector<Train> &obj)
{
    std::sort(obj.begin(), obj.end(), [](const Train &left, const Train &right)
              { return left.st.front().attribute("OraP").as_uint() < right.st.front().attribute("OraP").as_uint(); });
}

std::unordered_set<std::string> Command::match(const std::string &str, FindBy criteria)
{
    assert(criteria == FindBy::CITY || criteria == FindBy::TRAIN);
#ifndef FIND_THRESHOLD
    const unsigned MAX_SIZE_ADJUSTMENT = 4;
    const unsigned threshold = str.size() <= MAX_SIZE_ADJUSTMENT ? 1 : (str.size() / 4 + 1);
#else
    const unsigned threshold = FIND_THRESHOLD;
#endif
    std::unordered_set<std::string> out;

    std::string search = criteria == FindBy::CITY ? WordOperation::removeDiacritics(str) : str;

    for (const auto &ele : criteria == FindBy::CITY ? *cityNames : *trainNumbers)
        if (ele == search)
            return {WordOperation::removeDiacritics(ele)};
        else if ((WordOperation::distance(ele, search) <= threshold && ele != "") ||
                 (threshold > 1 && ele.find(search) != -1lu))
            out.insert(WordOperation::removeDiacritics(ele));

    return out;
}
