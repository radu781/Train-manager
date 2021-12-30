#pragma once

#include "pc.h"

class Types
{
public:
    template <typename T>
    static std::string toString(const T &val)
    {
        std::ostringstream out;
        out << val;
        return out.str();
    }
};
