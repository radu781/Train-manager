#pragma once

#include "pc.h"

class Command
{
public:
    virtual std::string execute() = 0;
    virtual std::string undo() = 0;

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
     * \return Vector of all matches
     */
    static std::unordered_set<std::string> match(const std::string &str, FindBy criteria);
    unsigned extractTime(const std::string &str);

    std::string findByCity(const std::string &how);

    using Stations = std::vector<pugi::xml_node>;
    struct Train
    {
        pugi::xml_node root;
        Stations st;
    };

    std::string getVerbose(const std::vector<Train> &obj);
    std::string getBrief(const std::vector<Train> &obj, bool needDelim = true, bool reverse = false);

    void sort(std::vector<Train> &obj);

    static pugi::xml_document *doc;
    static std::unordered_set<std::string> *cityNames;
    static std::unordered_set<std::string> *trainNumbers;

    static std::mutex m;

    std::vector<std::string> *command;

    static constexpr const char *OraS = "OraS", *OraP = "OraP";
    static constexpr const char *staOrig = "DenStaOrigine", *staDest = "DenStaDestinatie";
    static constexpr const char *trainOk = "[o] ", *trainNOk = "[x] ";
    static constexpr const char *CatTren = "CategorieTren", *Numar = "Numar";
};
