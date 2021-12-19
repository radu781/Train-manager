#include "pc.h"
#include <sstream>
#include "utils/wordoperation.hpp"

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
    std::vector<std::vector<int>> mat;
    mat.resize(2);
    mat[0].resize(src.size() + 1);
    mat[1].resize(src.size() + 1);

    for (int i = 0; i <= src.size(); i++)
        mat[0][i] = i;

    for (int i = 1; i <= dest.size(); i++)
        for (int j = 0; j <= src.size(); j++)
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
