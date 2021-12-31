#pragma once

class LoadingBar
{
public:
    LoadingBar(unsigned timeBetween, char empty = ' ', char full = '.');
    void update(unsigned seconds);

private:
    unsigned time;
    char empty;
    char full;
};
