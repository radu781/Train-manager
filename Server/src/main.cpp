#include <iostream>
#include "../include/connection.hpp"

int main()
{
    Connection *c;
    try
    {
        c = c->getInstance();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }

    int c1 = c->accept(), c2 = c->accept();
    c->send(c1, "hello client 1");
    c->send(c2, "hello client 2");

    return 0;
}
