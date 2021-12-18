#pragma once

#include <string>

class WordOperation
{
public:
    static std::string removeDiacritics(std::string str);
    static unsigned distance(const std::string &src, const std::string &dest);
};
