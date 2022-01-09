#include "pc.h"
#include "arrivals.hpp"

std::string Arrivals::execute()
{
    return findByCity(OraS);
}

std::string Arrivals::undo()
{
    return "arrivals undo";
}
