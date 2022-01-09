#pragma once

#include "pc.h"

class Command
{
public:
    virtual std::string execute() = 0;
    virtual std::string undo() = 0;

    void setCommand(std::vector<std::string> *command);
    void init(pugi::xml_document *doc, std::unordered_set<std::string> *cities, std::unordered_set<std::string>* trains);
    virtual ~Command() {}

    static pugi::xml_document *doc;
    static std::unordered_set<std::string> *cityNames;
    static std::unordered_set<std::string> *trainNumbers;

protected:
    using Stations = std::vector<pugi::xml_node>;
    struct Train
    {
        pugi::xml_node root;
        Stations st;
    };

    std::string getVerbose(const std::vector<Train> &obj);
    std::string getBrief(const std::vector<Train> &obj, bool needDelim = true, bool reverse = false);

    void sort(std::vector<Train> &obj);

    enum class FindBy
    {
        CITY,
        TRAIN
    };
    static std::unordered_set<std::string> match(const std::string &str, FindBy criteria);

    static std::mutex m;

    std::vector<std::string>* command;
    // friend class Connection;

    enum class CommandTypes
    {
        TODAY,
        DEPARTURES,
        ARRIVALS,
        LATE,
        HELP,

        COUNT,

        NOT_ENOUGH_ARGS,
        TOO_MANY_ARGS,
        NOT_FOUND
    };
    struct Args
    {
        unsigned mandatory;
        unsigned optional;
        CommandTypes type;
    };

    const std::unordered_map<std::string, Args> commands = {
        {"today", {2, -1u, CommandTypes::TODAY}},
        {"departures", {2, -1u, CommandTypes::DEPARTURES}},
        {"arrivals", {2, -1u, CommandTypes::ARRIVALS}},
        {"late", {2, -1u, CommandTypes::LATE}},
        {"help", {0, 1u, CommandTypes::HELP}}};

    static constexpr const char *OraS = "OraS", *OraP = "OraP";
    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *trainOk = "[o] ", *trainNOk = "[x] ";
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";
};
