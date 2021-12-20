#include "utils/types.hpp"

std::string Types::toString(const int val)
{
    char out[32];
    sprintf(out, "%d", val);
    return out;
}

std::string Types::toString(const unsigned val)
{
    char out[32];
    sprintf(out, "%u", val);
    return out;
}
