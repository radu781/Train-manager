#include "pc.h"
#include <ctime>
#include <sstream>
#include <communication/command.hpp>

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

std::pair<size_t, uint8_t> Command::validate()
{
    std::transform(command[0].begin(), command[0].end(), command[0].begin(),
                   tolower);

    if (command.size() >= (uint8_t)-1)
        return {-1, 0};

    for (size_t i = 0; i < (size_t)CommandTypes::COUNT; i++)
        if (command[0] == commands[i].command)
        {
            // if command with optional values
            if (commands[i].argCount > OPTIONAL_OFFSET)
            {
                if (command.size() - 1 <= commands[i].argCount - OPTIONAL_OFFSET)
                    return {i, command.size() - 1};
                else
                    // too many args
                    return {i, (uint8_t)Errors::TOO_MANY_ARGS};
            }
            // if command with fixed number of values
            else if (commands[i].argCount < OPTIONAL_OFFSET)
            {
                if (command.size() - 1 == commands[i].argCount)
                    return {i, command.size() - 1};
                else
                    // wrong arg count
                    return {i, (uint8_t)Errors::WRONG_ARG_COUNT};
            }
        }

    return {-1, 0};
}

std::string Command::execute()
{
    if (command.size() == 0)
        return "";

    auto [cmd, argc] = validate();
    if (cmd == (size_t)-1)
        return "Command " + command[0] + " is (currently) not supported";
    if (argc == (uint8_t)Errors::TOO_MANY_ARGS)
        return "Command " + command[0] + " has " +
               (char)(commands[cmd].argCount + '0' - OPTIONAL_OFFSET) +
               " optional arguments, " + (char)(command.size() - 1 + '0') + " provided";
    if (argc == (uint8_t)Errors::WRONG_ARG_COUNT)
        return "Command " + command[0] + " has " +
               (char)(commands[cmd].argCount + '0') + " arguments, " +
               (char)(command.size() - 1 + '0') + " provided";

    switch (cmd)
    {
    case (size_t)CommandTypes::TODAY:
        return Command::today();
    case (size_t)CommandTypes::DEPARTURES:
        return Command::departures();
    case (size_t)CommandTypes::ARRIVALS:
        return Command::arrivals();
    case (size_t)CommandTypes::HELP:
        return Command::help();
    default:
        LOG_DEBUG("Unexpected command " + command[0]);
        return "Try again";
    }
}

std::string Command::today()
{
    if (command[1] == command[2])
        return "Please enter different start and destinations";

    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();
    std::string out;
    std::string brief;
    int counter = 0;

    auto [start, dest] = split();
    if (start == "" && dest == "")
        return "Did not match any cities";

    time_t now = ::time(0);
    tm *ltm = localtime(&now);
    unsigned timeInSec = (ltm->tm_hour - 5) * 3600 + ltm->tm_min * 60 + ltm->tm_sec;

    const char *OraS = "OraS", *OraP = "OraP";
    const char *staOrig = "DenStaOrigine", *staDest= "DenStaDestinatie";

    unsigned available = 0;
    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();
        pugi::xml_node startNode, endNode;
        std::vector<pugi::xml_node> stations;

        // for each "Trasa" check if start and dest exist and occur in this order
        for (const auto &element : trasa)
        {
            if (normalize(element.attribute(staOrig).as_string())
                    .find(normalize(start)) != -1)
                startNode = element;
            if (normalize(element.attribute(staDest).as_string())
                        .find(normalize(dest)) != -1 &&
                !startNode.empty())
                endNode = element;

            if (!startNode.empty() && endNode.empty())
                stations.push_back(element);
        }

        const char *trainOk = "[o] ", *trainNOk = "[x] ";
        // If start and dest are found, recreate the path
        if (!endNode.empty() && !stations.empty())
        {
            char number[12]{};
            sprintf(number, "%d. ", ++counter);

            if (timeInSec < startNode.attribute(OraP).as_int())
            {
                available++;
                brief += trainOk;
            }
            else
                brief += trainNOk;
            brief += number +
                     std::string("(") +
                     getTime(startNode.attribute(OraP).as_int()) +
                     " -> " + getTime(endNode.attribute(OraS).as_int()) + ") " +
                     startNode.attribute(staOrig).as_string() + " -> " +
                     endNode.attribute(staDest).as_string() + "\n";

            strcat(number, "\n");
            out += number;
            for (const auto &station : stations)
                out += std::string("(") + getTime(station.attribute(OraP).as_int()) +
                       " -> " + getTime(station.attribute(OraP).as_int()) + ") " +
                       station.attribute(staOrig).as_string() + " -> " +
                       station.attribute(staDest).as_string() + "\n";
            out += "\n";
        }
    }
    if (!available)
        brief += "No trains could be found at this time";

    if (!counter)
        return "Found no trains from " + start + " to " + dest;

    char buff[32];
    if (counter == 1)
        sprintf(buff, "Found 1 train:\n");
    else
        sprintf(buff, "Found %d trains:\n", counter);
    return std::string(buff) + "---Verbose result:---\n" + out +
           "---Brief result:---\n" + brief;
}

bool Command::setContains(const std::string &str)
{
    for (const auto &ele : oraseFull)
        if (ele.find(str) != -1)
            return true;

    return false;
}

std::pair<std::string, std::string> Command::split()
{
    for (int i = 1; i < command.size() - 1; i++)
    {
        std::string start = command[1], dest = command[i + 1];
        for (int j = 2; j <= i; j++)
            start += " " + command[j];
        for (int j = i + 2; j < command.size(); j++)
            dest += " " + command[j];

        if (setContains(normalize(start)) && setContains(normalize(dest)))
            return {start, dest};
    }
    return {};
}

std::string Command::arrivals()
{
    return "";
}

std::string Command::departures()
{
    return "";
}

std::string Command::help()
{
    return "Supported commands:\n\
\ttoday [start] [dest] (get today's trains schedules from [start] to [dest])\n\
\tdepartures [start] [delta] (get the departures from [start] in the upcoming\
 hour, if delta is specified, it will get the departures in the upcoming delta\
 minutes)\n\
\tarrivals [dest] [delta] (get the arrivals to [dest] in the upcoming hour, if\
 delta is specified, it will get the departures in the upcoming delta minutes)\n\
 \t (quit close the connection)";
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
            orase.insert(ele.attribute("DenStaOrigine").as_string());
            orase.insert(ele.attribute("DenStaDestinatie").as_string());
        }
    }

    for (const auto &ele : orase)
        oraseFull.push_back(normalize(ele));
}

std::string Command::normalize(std::string str)
{
    static const std::unordered_map<const char *, char> dict = {
        {"ă", 'a'},
        {"â", 'a'},
        {"â", 'a'},
        {"î", 'i'},
        {"ș", 's'},
        {"ş", 's'},
        {"ț", 't'},
        {"ţ", 't'}};

    std::istringstream in(str);
    std::string token;
    str.clear();
    std::for_each(str.begin(), str.end(), [](char &c)
                  { c = std::tolower(c); });
    while (std::getline(in, token, ' '))
        str += (char)std::toupper(token[0]) + token.substr(1) + " ";

    for (const auto &[key, val] : dict)
    {
        size_t pos = str.find(key);
        while (pos != std::string::npos)
        {
            str.replace(pos, 2, 1, val);
            pos = str.find(key);
        }
    }

    return str;
}

std::string Command::getTime(int seconds)
{
    char buff[16];
    sprintf(buff, "%.2d:%.2d", seconds / 3600, seconds % 3600 / 60);
    return buff;
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
