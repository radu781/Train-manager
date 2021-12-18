#include "pc.h"
#include <sstream>
#include <utils/wordoperation.hpp>

std::string WordOperation::removeDiacritics(std::string str)
{
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

    return str;
}

unsigned WordOperation::distance(const std::string &src, const std::string &dest)
{
    int len1 = src.size();
    int len2 = dest.size();

    int DP[2][len1 + 1];

    memset(DP, 0, sizeof DP);

    for (int i = 0; i <= len1; i++)
        DP[0][i] = i;

    for (int i = 1; i <= len2; i++)
    {
        for (int j = 0; j <= len1; j++)
        {
            if (j == 0)
                DP[i % 2][j] = i;

            else if (src[j - 1] == dest[i - 1])
                DP[i % 2][j] = DP[(i - 1) % 2][j - 1];

            else
                DP[i % 2][j] = 1 + std::min(DP[(i - 1) % 2][j],
                                            std::min(DP[i % 2][j - 1],
                                                     DP[(i - 1) % 2][j - 1]));
        }
    }

    return DP[len2 % 2][len1];
}
