#include <algorithm>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <utility>
#include <communication/command.hpp>
#include <utils/logger.hpp>
#include <xml/pugixml.hpp>

pugi::xml_document Command::doc{};

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
    getFile();
    auto trenuri = doc.child("XmlIf").child("XmlMts").child("Mt").child("Trenuri").children();
    std::string out;
    for (const auto &tren : trenuri)
    {
        auto trasa = tren.child("Trase").child("Trasa").children();
        bool foundStart = false, foundDest = false;
        pugi::xml_node start, end;
        std::string temp;

        for (const auto &element : trasa)
        {
            if (element.attribute("DenStaOrigine").as_string() == command[1])
                start = element;
            if (element.attribute("DenStaDestinatie").as_string() == command[2])
                end = element;
        }
        if (!start.empty() && !end.empty())
            out += start.attribute("DenStaOrigine").as_string() +
                   std::string("(") +
                   getTime(start.attribute("OraP").as_int()) +
                   std::string(") to ") +
                   end.attribute("DenStaDestinatie").as_string() +
                   std::string("(") +
                   getTime(end.attribute("OraS").as_int()) +
                   ")\n";
    }
    return out;
}

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
 delta is specified, it will get the departures in the upcoming delta minutes)";
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
    case 7:
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
