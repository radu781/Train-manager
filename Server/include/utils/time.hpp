#pragma once

#include "pc.h"

class Time
{
public:
    static unsigned current();
    static std::string toString(unsigned seconds);
    static unsigned diff(unsigned endTime, unsigned startTime);
    static bool isBetween(unsigned time1, unsigned time2);

private:
    static const unsigned SECONDS_IN_DAY = 3600 * 24;
};
