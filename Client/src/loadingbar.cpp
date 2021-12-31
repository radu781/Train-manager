#include <chrono>
#include <iostream>
#include <string.h>
#include <thread>
#include "loadingbar.hpp"

LoadingBar::LoadingBar(unsigned timeBetween, char empty, char full)
    : time(timeBetween), empty(empty), full(full) {}

void LoadingBar::update(unsigned seconds)
{
    char bar[64] = "[";
    for (unsigned j = 0; j < seconds; j++)
        bar[j + 1] = ' ';
    bar[seconds + 1] = ']';
    for (unsigned j = 0; j < seconds; j++)
    {
        std::cout << bar << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (unsigned k = 0; k < seconds + 2; k++)
            std::cout << '\b';
        bar[j + 1] = full;
    }
    std::cout << bar << std::endl;
}
