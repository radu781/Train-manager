#include <iostream>
#include "../include/connection.hpp"

int main()
{
    Connection *c = c->getInstance();
    try
    {
        c->makeConnection();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }

    for (;;)
    {
        try
        {
            std::cout << c->read(c->client.sock) << '\n';
            std::string sender = "hello from server";
            c->send(c->client.sock, sender);
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << '\n';
        }
    }

    return 0;
}
