#pragma once

#include <string>

#define LPAD(str, total) WordOperation::pad(str, total, WordOperation::Pad::Left)
#define CPAD(str, total) WordOperation::pad(str, total, WordOperation::Pad::Center)
#define RPAD(str, total) WordOperation::pad(str, total, WordOperation::Pad::Right)

class WordOperation
{
public:
    static std::string removeDiacritics(std::string str);
    static unsigned distance(const std::string &src, const std::string &dest);

    enum Pad
    {
        Left,
        Center,
        Right
    };
    static std::string pad(std::string str, unsigned total, Pad pad);
};
