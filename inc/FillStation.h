#pragma once
#include "tui.h"
#include <stdint.h>


class FillStation{
    private :
    uint8_t state;
    public:

    FillStation();
    void draw(WINDOW* w);
    ~FillStation();
};
