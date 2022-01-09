#include "help.hpp"

std::string Help::execute()
{
    if (command->size() == 1)
        return "Supported commands:\n\
\ttoday [start] [dest] (get today's trains schedules from [start] to [dest])\n\
\tdepartures [start] [delta] (get the departures from [start] in the upcoming\
 hour, if delta is specified, it will get the departures in the upcoming delta\
 minutes)\n\
\tarrivals [dest] [delta] (get the arrivals to [dest] in the upcoming hour, if\
 delta is specified, it will get the departures in the upcoming delta minutes)\
\n\tquit (close the connection)\n\
\nhelp [command] (get more detailed help about a command)";

    if (commands.find(command->at(1)) == commands.end())
        return "Command " + command->at(1) + " not found";

    auto cmd = commands.at(command->at(1));
    switch (cmd.type)
    {
    case CommandTypes::TODAY:
        return "The today command returns all trains from a starting point to \
an end point.\nIt has " +
               Types::toString<int>(cmd.mandatory) + " mandatory arguments \
(at least a starting and destination city are needed). City names are \
automatically detected, there is no need to separate them, for example\n\t\
today cluj napoca iasi\nwill return all trains from cluj napoca to iasi.";

    case CommandTypes::DEPARTURES:
        return "The departures command returns all departures from a starting \
point in the next delta minutes.\nIt has " +
               Types::toString<int>(cmd.mandatory) +
               " mandatory arguments (a starting city and a time in minutes). \
You can either use numbers for seconds or literals for more accurate queries. \
The accepted literals are \"s\", \"m\", \"h\" for seconds, minutes, hours \
respectively. Example usage:\n\tdepartures iasi 10m\nwill return the trains \
that depart from iasi in the following 10 minutes.";

    case CommandTypes::ARRIVALS:
        return "The arrivals command returns all arrivals from a starting \
point in the next delta minutes.\nIt has " +
               Types::toString<int>(cmd.mandatory) +
               " mandatory arguments (a starting city and a time in minutes). \
Example usage:\n\tarrivals iasi 15\nwill return the trains that arrive in iasi \
in the following 15 minutes.";

    case CommandTypes::LATE:
        return "Not yet implemented";

    case CommandTypes::HELP:
        return "Why would you need help about a help command?";

    default:
        LOG_DEBUG("Unexpected " + command->at(1) + " type: " +
                  Types::toString<unsigned>(unsigned(cmd.type)));
        return "Try again";
    }
}

std::string Help::undo()
{
    return "help undo";
}
