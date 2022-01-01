#include "pc.h"
#include <sstream>
#include "utils/wordoperation.hpp"

std::string WordOperation::removeDiacritics(std::string str)
{
    if (str == "")
        return "";
    const std::unordered_map<const char *, char> dict = {
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

    for (size_t i = 0; i < str.size() - 1; i++)
        if (ispunct(str[i]) && islower(str[i + 1]))
            str[i + 1] = toupper(str[i + 1]);

    return str.substr(0, str.size() - 1);
}

unsigned WordOperation::distance(const std::string &src, const std::string &dest)
{
    std::vector<std::vector<int>> mat;
    mat.resize(2);
    mat[0].resize(src.size() + 1);
    mat[1].resize(src.size() + 1);

    for (size_t i = 0; i <= src.size(); i++)
        mat[0][i] = i;

    for (size_t i = 1; i <= dest.size(); i++)
        for (size_t j = 0; j <= src.size(); j++)
        {
            if (j == 0)
                mat[i % 2][j] = i;

            else if (src[j - 1] == dest[i - 1])
                mat[i % 2][j] = mat[(i - 1) % 2][j - 1];

            else
                mat[i % 2][j] = 1 + std::min(mat[(i - 1) % 2][j], std::min(mat[i % 2][j - 1],
                                                                         mat[(i - 1) % 2][j - 1]));
        }

    return mat[dest.size() % 2][src.size()];
}

std::string WordOperation::pad(std::string str, unsigned total, Pad pad)
{
    assert(str.size() <= total);
    std::string spaces;
    for (size_t i = 0; i < total - str.size(); i++)
        spaces += " ";

    switch (pad)
    {
    case Pad::Left:
        return spaces + str;
    case Pad::Center:
        return spaces.substr(spaces.size() / 2) + str + spaces.substr(spaces.size() / 2);
    case Pad::Right:
        return str + spaces;
    default:
        return "Unknown pad";
    }
}

std::string WordOperation::trim(std::string str)
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
