#pragma once

#include "pc.h"

class Time
{
public:
    static unsigned current();
    static std::string toString(unsigned seconds);
    static unsigned diff(unsigned endTime, unsigned startTime);
    static std::string diffToString(unsigned endTime, unsigned startTime);
    static bool isBetween(unsigned time1, unsigned time2);
    static bool isBetween(unsigned time1, unsigned who, unsigned time2);

    static const unsigned SECONDS_IN_DAY = 3600 * 24;
    static const unsigned MINUTES_IN_DAY = 60 * 24;
};
