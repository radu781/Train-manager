#include "pc.h"
#include <ctime>
#include "communication/command.hpp"
#include "utils/wordoperation.hpp"
#include "utils/time.hpp"

pugi::xml_document Command::doc{};
std::unordered_set<std::string> Command::orase;
std::vector<std::string> Command::oraseFull;

Command::Command(const std::string &str)
{
    if (str == "")
        return;

    std::string trimmed = trim(str);
    std::istringstream in(trimmed);
    std::string token;
    // TODO: add more delims in the future
    while (std::getline(in, token, ' '))
        command.push_back(token);
}

Command::CommandTypes Command::validate()
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

std::string Command::execute()
{
    if (command.size() == 0)
        return "";

    try
    {
        auto cmd = commands.at(command[0]);

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
            return today();
        case CommandTypes::DEPARTURES:
            return departures();
        case CommandTypes::ARRIVALS:
            return arrivals();
        case CommandTypes::LATE:
            return late();
        case CommandTypes::HELP:
            return help();

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

std::string Command::today()
{
    if (command[1] == command[2])
        return "Please enter different start and destinations";

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    std::vector<std::string> startVec, destVec;
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

            if (contains(startVec, testStart)) // only the newest one
                startNode = element;
            if (!startNode.empty())
                stations.push_back(element);

            if (contains(destVec, testDest))
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

    Command::sort(unsorted);
    return getVerbose(unsorted) + "\n" + getBrief(unsorted);
}

std::vector<std::string> Command::find(const std::string &str)
{
    const unsigned MAX_SIZE_ADJUSTMENT = 5;
    const unsigned threshold = str.size() <= MAX_SIZE_ADJUSTMENT ? 2 : (str.size() / 4 + 1);
    std::vector<std::string> out;

    for (const auto &ele : oraseFull)
        if ((ele == str || ele.find(str) != -1lu ||
             WordOperation::distance(ele, str) <= threshold) &&
            ele != "")
            out.push_back(WordOperation::removeDiacritics(ele));

    return out;
}

bool Command::contains(const std::vector<std::string> &vec, const std::string &str)
{
    for (const auto &s : vec)
        if (s == str)
            return true;
    return false;
}

std::pair<std::vector<std::string>, std::vector<std::string>> Command::splitNames()
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

        auto startVec = find(WordOperation::removeDiacritics(start));
        auto endVec = find(WordOperation::removeDiacritics(dest));
        if (!startVec.empty() && !endVec.empty())
            return {startVec, endVec};
    }
    return {};
}

std::string Command::findByCity(const std::string &timeType)
{
    assert(timeType == OraS || timeType == OraP);
    std::string wholeCity;
    for (size_t i = 1; i < command.size() - 1; i++)
        wholeCity += command[i];
    wholeCity = WordOperation::removeDiacritics(wholeCity);

    std::vector<std::string> matching = find(wholeCity);
    if (matching.empty())
        return "Please enter a valid city";

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    int delta = atoi(command.back().c_str());
    if (delta < 0)
        return "Please try not to use negative values";
    if ((unsigned)delta > Time::MINUTES_IN_DAY)
        return "Please use the \"today\" command instead for timetables further than a day";

    std::vector<Train> stations;

    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();

        for (const auto &element : trasa)
        {
            unsigned timeStamp = element.attribute(timeType.c_str()).as_int();
            std::string cityStr = WordOperation::removeDiacritics(element.attribute(staOrig).as_string());

            if (contains(matching, cityStr))
            {
                if (timeType == OraS && Time::isBetween(timeStamp - 60 * delta, timeStamp))
                    stations.push_back({tren, std::vector<pugi::xml_node>{element}});
                else if (timeType == OraP && Time::isBetween(timeStamp - 60 * delta, timeStamp))
                    stations.push_back({tren, std::vector<pugi::xml_node>{element}});
            }
        }
    }
    if (stations.empty())
        return "Could not find any " + std::string(timeType == OraP ? "departures" : "arrivals") +
               " in the upcoming " + Types::toString<int>(delta) + " minutes";
    sort(stations);
    return getBrief(stations, false, timeType == std::string(OraS));
}

std::string Command::arrivals()
{
    return findByCity(OraS);
}

std::string Command::departures()
{
    return findByCity(OraP);
}

std::string Command::late()
{
    return "late call";
}

std::string Command::help()
{
    if (command.size() == 1)
        return "Supported commands:\n\
\ttoday [start] [dest] (get today's trains schedules from [start] to [dest])\n\
\tdepartures [start] [delta] (get the departures from [start] in the upcoming\
 hour, if delta is specified, it will get the departures in the upcoming delta\
 minutes)\n\
\tarrivals [dest] [delta] (get the arrivals to [dest] in the upcoming hour, if\
 delta is specified, it will get the departures in the upcoming delta minutes)\
\n\tquit (close the connection)\n\
\nhelp [command] (get more detailed help about a command)";

    if (commands.find(command[1]) == commands.end())
        return "Command " + command[1] + " not found";

    auto cmd = commands.at(command[1]);
    switch (cmd.type)
    {
    case CommandTypes::TODAY:
        return "The today command returns all trains from a starting point to \
an end point.\nIt has " +
               Types::toString<int>(cmd.mandatory) + " mandatory arguments \
(at least a starting and destination city are needed). City names are \
automatically detected, there is no need to separate them, for example\n\t\
today cluj napoca iasi\nwill return all trains from cluj napoca to iasi.";

    case CommandTypes::DEPARTURES:
        return "The departures command returns all departures from a starting \
point in the next delta minutes.\nIt has " +
               Types::toString<int>(cmd.mandatory) +
               " mandatory arguments (a starting city and a time in minutes). \
Example usage:\n\tdepartures iasi 10\nwill return the trains that depart from \
iasi in the following 10 minutes.";

    case CommandTypes::ARRIVALS:
        return "The arrivals command returns all arrivals from a starting \
point in the next delta minutes.\nIt has " +
               Types::toString<int>(cmd.mandatory) +
               " mandatory arguments (a starting city and a time in minutes). \
Example usage:\n\tarrivals iasi 15\nwill return the trains that arrive in iasi \
in the following 15 minutes.";

    case CommandTypes::LATE:
        return "Not yet implemented";

    case CommandTypes::HELP:
        return "Why would you need help about a help command?";

    default:
        LOG_DEBUG("Unexpected " + command[1] + " type: " + Types::toString<unsigned>(unsigned(cmd.type)));
        return "Try again";
    }
}

std::string Command::motd()
{
    time_t now = ::time(0);
    tm *ltm = localtime(&now);

    char buff[128], weekDay[12];
    switch (ltm->tm_wday)
    {
    case 1:
        strcpy(weekDay, "Monday");
        break;
    case 2:
        strcpy(weekDay, "Tuesday");
        break;
    case 3:
        strcpy(weekDay, "Wednesday");
        break;
    case 4:
        strcpy(weekDay, "Thursday");
        break;
    case 5:
        strcpy(weekDay, "Friday");
        break;
    case 6:
        strcpy(weekDay, "Saturday");
        break;
    case 0:
        strcpy(weekDay, "Sunday");
        break;
    default:
        strcpy(weekDay, "???");
        break;
    }
    sprintf(buff, "---Welcome to Train Manager---\n\
Today is %d/%d/%d(%s)",
            ltm->tm_mday, ltm->tm_mon + 1, ltm->tm_year + 1900, weekDay);

    return buff;
}

void Command::getFile()
{
    const std::string localPath = "resources/cfr_2021.xml";
    const std::string web = "https://data.gov.ro/dataset/c4f71dbb-de39-49b2-b697-5b60a5f299a2/resource/5af0366b-f9cb-4d6e-991b-e91789fc7d2c/download/sntfc-cfr-cltori-s.a.-1232-trenuri_2021.xml ";
    const std::string args = "--tries=3 -O " + localPath;

    if (!std::filesystem::exists(localPath))
    {
        std::mutex m;
        m.lock();

        LOG_DEBUG("Xml does not exist locally, attempting to download");
        if (system(("wget " + web + args).c_str()) < 0)
            LOG_DEBUG("Xml failed to download");
        else
            LOG_DEBUG("Xml downloaded");

        m.unlock();
    }

    LOG_DEBUG("Loaded xml");
    doc.load_file(localPath.c_str());

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();

    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();
        for (const auto &ele : trasa)
        {
            orase.insert(ele.attribute(staOrig).as_string());
            orase.insert(ele.attribute(staDest).as_string());
        }
    }

    for (const auto &ele : orase)
        oraseFull.push_back(WordOperation::removeDiacritics(ele));
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
        if (reverse)
            std::swap(start, end);

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

std::string Command::trim(std::string str)
{
    if (str == "")
        return "";

    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c)
                                        { return !std::isspace(c); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char c)
                           { return !std::isspace(c); })
                  .base(),
              str.end());

    return str;
}
