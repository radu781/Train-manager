#include "pc.h"
#include "utils/time.hpp"

unsigned Time::current()
{
    time_t now = ::time(0);
    tm *ltm = localtime(&now);
    unsigned timeInSec = ltm->tm_hour * 3600 + ltm->tm_min * 60 + ltm->tm_sec;

    return timeInSec;
}

std::string Time::toString(unsigned seconds)
{
    char buff[16];
    sprintf(buff, "%.2d:%.2d", seconds / 3600, seconds % 3600 / 60);
    return buff;
}

unsigned Time::diff(unsigned endTime, unsigned startTime)
{
    if (startTime > endTime)
        return endTime + SECONDS_IN_DAY - startTime;
    return endTime - startTime;
}

bool Time::isBetween(unsigned time1, unsigned time2)
{
    return (current() > time1 && current() < time2) ||
           (current() > time2 && current() < time1);
}
