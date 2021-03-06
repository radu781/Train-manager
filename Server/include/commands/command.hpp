#pragma once

#include "pc.h"

class Command
{
public:
    virtual std::string execute() = 0;
    virtual std::string undo() = 0;

    Command();
    Command(const Command *other, const std::vector<std::string> *command);

    void setCommand(std::vector<std::string> *command);
    void init(pugi::xml_document *doc, std::unordered_set<std::string> *cities,
              std::unordered_set<std::string> *trains);
    virtual ~Command() {}

protected:
    enum class FindBy
    {
        CITY,
        TRAIN
    };

    /**
     * @brief Check if the unordered_set contains the string
     *
     * \param str String to be searched for in the string. Substrings are
     * considered valid too
     * \return Unordered set of all matches
     */
    static std::unordered_set<std::string> match(const std::string &str, FindBy criteria);
    unsigned extractTime(const std::string &str);

    using Stations = std::vector<pugi::xml_node>;
    struct Train
    {
        pugi::xml_node root;
        Stations st;
    };

    std::string getVerbose(const std::vector<Train> &obj);
    std::string getBrief(const std::vector<Train> &obj, bool needDelim = true);

    enum class Criteria
    {
        DEPARTURE,
        ARRIVAL
    };

    void sort(std::vector<Train> &obj, Criteria c);

    static pugi::xml_document *doc;
    static std::unordered_set<std::string> *cityNames;
    static std::unordered_set<std::string> *trainNumbers;

    static std::mutex m;

    const std::vector<std::string> *command;

    static constexpr const char *OraS = "OraS", *OraP = "OraP";
    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *trainOk = "[o] ", *trainNOk = "[x] ";
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";

    static constexpr const char *cannotUndo = "Nothing to undo here, this command only prints information";
};
