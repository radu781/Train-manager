#include "pc.h"
#include "communication/commandparser.hpp"
#include "commands/arrivals.hpp"
#include "commands/departures.hpp"
#include "commands/help.hpp"
#include "commands/late.hpp"
#include "commands/today.hpp"
#include "communication/client.hpp"

pugi::xml_document CommandParser::doc{};
std::unordered_set<std::string> CommandParser::cityNames;
std::unordered_set<std::string> CommandParser::trainNumbers;
std::mutex CommandParser::m;
Command *CommandParser::sharedCmd = nullptr;
const std::unordered_map<std::string, CommandParser::Args> CommandParser::commandRules = {
    {"today", {2, -1u, CommandTypes::TODAY}},
    {"departures", {2, -1u, CommandTypes::DEPARTURES}},
    {"arrivals", {2, -1u, CommandTypes::ARRIVALS}},
    {"late", {2, -1u, CommandTypes::LATE}},
    {"help", {0, 1u, CommandTypes::HELP}},
};

CommandParser::CommandParser(const std::string &str)
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

CommandParser::CommandTypes CommandParser::validate()
{
    std::transform(command[0].begin(), command[0].end(), command[0].begin(),
                   tolower);

    if (commandRules.contains(command[0]))
    {
        Args pos = commandRules.at(command[0]);

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

std::string CommandParser::execute(Client *client)
{
    if (command.size() == 0)
        return "";

    try
    {
        auto cmd = commandRules.at(command[0]);
        std::string out;
        Command *icmd = client->cmd;

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
            icmd = new Today(CommandParser::sharedCmd, &command);
            out = icmd->execute();
            delete icmd;
            return out;

        case CommandTypes::DEPARTURES:
            icmd = new Departures(CommandParser::sharedCmd, &command);
            out = icmd->execute();
            delete icmd;
            return out;

        case CommandTypes::ARRIVALS:
            icmd = new Arrivals(CommandParser::sharedCmd, &command);
            out = icmd->execute();
            delete icmd;
            return out;

        case CommandTypes::LATE:
            icmd = new Late(CommandParser::sharedCmd, &command);
            out = icmd->execute();
            delete icmd;
            return out;

        case CommandTypes::HELP:
            icmd = new Help(CommandParser::sharedCmd, &command);
            out = icmd->execute();
            delete icmd;
            return out;

        default:
            LOG_DEBUG("Unexpected command " + command[0]);
            return "Try again";
        }
    }
    catch (const std::out_of_range &e) // caused by the commandRules.at()
    {
        return "Command " + command[0] + " not found";
    }
}

void CommandParser::getFile()
{
    const std::string localPath = "resources/cfr_2021.xml";
    const std::string web = "https://data.gov.ro/dataset/c4f71dbb-de39-49b2-b697-5b60a5f299a2/resource/5af0366b-f9cb-4d6e-991b-e91789fc7d2c/download/sntfc-cfr-cltori-s.a.-1232-trenuri_2021.xml ";
    const std::string args = "--tries=3 -O " + localPath;

    if (!std::filesystem::exists(localPath))
    {
        std::scoped_lock<std::mutex> lock(m);

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

    sharedCmd->init(&doc, &cityNames, &trainNumbers);
}
