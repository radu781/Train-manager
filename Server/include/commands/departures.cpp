#include "pc.h"
#include "departures.hpp"

std::string Departures::execute()
{
    return findByCity(OraP);
}

std::string Departures::undo()
{
    return "departures undo";
}
