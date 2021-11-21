#include <iostream>
#include "../include/connection.hpp"

int main()
{
    Connection *c = c->getInstance();
    c->makeConnection();
    for (;;)
    {
        c->send("hello from client");
        std::cout << c->read() << '\n';
        // simulating some delay
        for (volatile int i = 0; i < 100000000; i++)
            ;
    }

    c->closeConnection();
    return 0;
}
