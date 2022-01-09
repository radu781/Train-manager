#include "pc.h"
#include <ctime>
#include "communication/commandgeneral.hpp"
#include "utils/wordoperation.hpp"
#include "utils/time.hpp"

#include "commands/help.hpp"
#include "commands/today.hpp"

pugi::xml_document CommandGeneral::doc{};
std::unordered_set<std::string> CommandGeneral::cityNames;
std::unordered_set<std::string> CommandGeneral::trainNumbers;
std::mutex CommandGeneral::m;
Command *CommandGeneral::icmd;

CommandGeneral::CommandGeneral(const std::string &str)
{
    if (str == "")
        return;

    std::string trimmed = WordOperation::trim(str);

    char *cstr = new char[trimmed.size() + 1]{};
    trimmed.copy(cstr, trimmed.size());
    const char *delim = " ,;'?";
    char *ptr = strtok(cstr, delim);
    while (ptr != nullptr)
    {
        command.push_back(ptr);
        ptr = strtok(NULL, delim);
    }
    delete[] cstr;
}

CommandGeneral::CommandTypes CommandGeneral::validate()
{
    std::transform(command[0].begin(), command[0].end(), command[0].begin(),
                   tolower);

    if (commands.contains(command[0]))
    {
        auto pos = commands.at(command[0]);

        if (command.size() - 1 < pos.mandatory)
            return CommandTypes::NOT_ENOUGH_ARGS;
        if (pos.optional != -1u &&
            command.size() - 1 > pos.mandatory + pos.optional)
            return CommandTypes::TOO_MANY_ARGS;

        if (command.size() - 1 >= pos.mandatory &&
            (pos.optional == -1u || command.size() - 1 <= pos.mandatory + pos.optional))
            return pos.type;
    }

    return CommandTypes::NOT_FOUND;
}

std::string CommandGeneral::execute()
{
    if (command.size() == 0)
        return "";

    try
    {
        auto cmd = commands.at(command[0]);
        std::string out;

        switch (validate())
        {
        case CommandTypes::NOT_ENOUGH_ARGS:
            return "Command " + command[0] + " has " +
                   Types::toString<unsigned>(cmd.mandatory) + " mandatory arguments, " +
                   Types::toString<unsigned>(command.size() - 1u) + " provided";
        case CommandTypes::TOO_MANY_ARGS:
            return "Command " + command[0] + " has up to " +
                   Types::toString<unsigned>(cmd.mandatory + cmd.optional) + " arguments, " +
                   Types::toString<unsigned>(command.size() - 1u) + " provided";
        case CommandTypes::NOT_FOUND:
            return "Command " + command[0] + " not found";

        case CommandTypes::TODAY:
            icmd = new Today();
            icmd->setCommand(&command);
            out = icmd->execute();
            delete icmd;
            return out;
        case CommandTypes::DEPARTURES:
            return departures();
        case CommandTypes::ARRIVALS:
            return arrivals();
        case CommandTypes::LATE:
            return late();
        case CommandTypes::HELP:
            icmd = new Help();
            icmd->setCommand(&command);
            out = icmd->execute();
            delete icmd;
            return out;

        default:
            LOG_DEBUG("Unexpected command " + command[0]);
            return "Try again";
        }
    }
    catch (const std::out_of_range &e) // caused by the commands.at()
    {
        return "Command " + command[0] + " not found";
    }
}

std::unordered_set<std::string> CommandGeneral::match(const std::string &str, FindBy criteria)
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

    for (const auto &ele : criteria == FindBy::CITY ? cityNames : trainNumbers)
        if (ele == search)
            return {WordOperation::removeDiacritics(ele)};
        else if ((WordOperation::distance(ele, search) <= threshold && ele != "") ||
                 (threshold > 1 && ele.find(search) != -1lu))
            out.insert(WordOperation::removeDiacritics(ele));

    return out;
}

std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> CommandGeneral::splitNames()
{
    const unsigned MIN_LENGTH_THRESHOLD = 3;
    for (size_t i = 1; i < command.size() - 1; i++)
        if (command[i].size() < MIN_LENGTH_THRESHOLD)
            throw SearchNotRefined("");

    for (size_t i = 1; i < command.size() - 1; i++)
    {
        std::string start = command[1], dest = command[i + 1];
        for (size_t j = 2; j <= i; j++)
            start += " " + command[j];
        for (size_t j = i + 2; j < command.size(); j++)
            dest += " " + command[j];

        auto startVec = match(WordOperation::removeDiacritics(start), FindBy::CITY);
        auto endVec = match(WordOperation::removeDiacritics(dest), FindBy::CITY);
        if (!startVec.empty() && !endVec.empty())
            return {startVec, endVec};
    }
    return {};
}

std::string CommandGeneral::findByCity(const std::string &timeType)
{
    assert(timeType == OraS || timeType == OraP);
    std::string wholeCity;
    for (size_t i = 1; i < command.size() - 1; i++)
        wholeCity += command[i];
    wholeCity = WordOperation::removeDiacritics(wholeCity);

    std::unordered_set<std::string> matching = match(wholeCity, FindBy::CITY);
    if (matching.empty())
        return "Please enter a valid city";

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    unsigned delta = extractTime(command.back());
    if (delta == -1u)
        return "Please try not to use negative values";
    if (delta == -2u)
        return "Please use some digits too";
    if (delta == -3u)
        return "Please use only number or literals as shown in \"help departures\"";
    if ((unsigned)delta > Time::SECONDS_IN_DAY)
        return "Please use the \"today\" command instead for timetables further than a day";

    std::vector<Train> stations;

    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();

        for (const auto &element : trasa)
        {
            unsigned timeStamp = element.attribute(timeType.c_str()).as_uint();
            std::string start = WordOperation::removeDiacritics(element.attribute(staOrig).as_string());
            std::string dest = WordOperation::removeDiacritics(element.attribute(staDest).as_string());

            if (matching.contains(start))
            {
                if (timeType == OraS && dest != start &&
                    Time::isBetween(Time::current(), timeStamp, Time::current() + delta))
                {
                    if (dest != start)
                        stations.push_back({tren, std::vector<pugi::xml_node>{element}});
                }
                else if (timeType == OraP && dest != start &&
                         Time::isBetween(Time::current(), timeStamp, Time::current() + delta))
                {
                    if (dest != start)
                        stations.push_back({tren, std::vector<pugi::xml_node>{element}});
                }
            }
        }
    }
    if (stations.empty())
        return "Could not find any " + std::string(timeType == OraP ? "departures" : "arrivals") +
               " in the upcoming " + Types::toString<int>(delta / 60) + " minutes";
    sort(stations);
    return getBrief(stations, false, timeType == std::string(OraS));
}

unsigned CommandGeneral::extractTime(const std::string &str)
{
    if (str.find('-') != -1lu)
        return -1u;
    if (std::none_of(str.begin(), str.end(), isdigit))
        return -2u;

    const char *literals = "smhSMH";
    if (str.find_first_of(literals) == -1lu)
        return atoi(str.c_str());

    unsigned seconds = 0, minutes = 0, hours = 0;
    for (size_t last = 0, next = str.find_first_of(literals); next != -1lu;
         last = next + 1, next = str.find_first_of(literals, last))
    {
        std::string ele = str.substr(last, next - last);
        LOG_DEBUG(ele);

        switch (str[next])
        {
        case 's':
        case 'S':
            seconds += atoi(ele.c_str());
            break;
        case 'm':
        case 'M':
            minutes += atoi(ele.c_str());
            break;
        case 'h':
        case 'H':
            hours += atoi(ele.c_str());
            break;
        default:
            LOG_DEBUG("Unexpected literal " + ele + " " + ele[next]);
        }
    }

    if (hours + minutes + seconds == 0)
        return -3u;
    return hours * 3600 + minutes * 60 + seconds;
}

std::string CommandGeneral::arrivals()
{
    return findByCity(OraS);
}

std::string CommandGeneral::departures()
{
    return findByCity(OraP);
}

std::string CommandGeneral::late()
{
    std::string trainName;
    for (size_t i = 1; i < command.size() - 1; i++)
        trainName += command[i] + " ";
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
        return "Found train number: " + command[1];
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

void CommandGeneral::getFile()
{
    const std::string localPath = "resources/cfr_2021.xml";
    const std::string web = "https://data.gov.ro/dataset/c4f71dbb-de39-49b2-b697-5b60a5f299a2/resource/5af0366b-f9cb-4d6e-991b-e91789fc7d2c/download/sntfc-cfr-cltori-s.a.-1232-trenuri_2021.xml ";
    const std::string args = "--tries=3 -O " + localPath;

    if (!std::filesystem::exists(localPath))
    {
        std::lock_guard<std::mutex> lock(m);

        LOG_DEBUG("Xml does not exist locally, attempting to download");
        if (!std::filesystem::exists("resources/"))
            std::filesystem::create_directory("resources/");
        if (system(("wget " + web + args).c_str()) < 0)
            LOG_DEBUG("Xml failed to download");
        else
            LOG_DEBUG("Xml downloaded");
    }

    LOG_DEBUG("Loaded xml");
    doc.load_file(localPath.c_str());

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    std::unordered_set<std::string> tmpCities;
    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();
        for (const auto &ele : trasa)
        {
            tmpCities.insert(ele.attribute(staOrig).as_string());
            tmpCities.insert(ele.attribute(staDest).as_string());
        }

        const std::string categ = tren.attribute(CatTren).as_string();
        const std::string number = tren.attribute(Numar).as_string();
        trainNumbers.insert(categ + number);
    }

    for (const auto &ele : tmpCities)
        cityNames.insert(WordOperation::removeDiacritics(ele));
    icmd->init(&doc, &cityNames, &trainNumbers);
}

std::string CommandGeneral::getVerbose(const std::vector<Train> &obj)
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

std::string CommandGeneral::getBrief(const std::vector<Train> &obj, bool needDelim, bool reverse)
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

void CommandGeneral::sort(std::vector<Train> &obj)
{
    std::sort(obj.begin(), obj.end(), [](const Train &left, const Train &right)
              { return left.st.front().attribute("OraP").as_uint() < right.st.front().attribute("OraP").as_uint(); });
}
