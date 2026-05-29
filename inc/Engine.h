#pragma once
#include "tui.h"
#include <stdint.h>


class Engine{
    private :
    uint8_t state;
    public:

    Engine();
    void draw(WINDOW* w);
    ~Engine();
};
