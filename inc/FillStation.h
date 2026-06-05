#pragma once
#include "tui.h"
#include <stdint.h>


#include <atomic>

class FillStation{
    private :
    std::atomic<uint8_t> state;
    public:

    FillStation();
    void draw(WINDOW* w, bool focused = false, bool connected = true);
    uint8_t getState() const { return state; }
    void setState(uint8_t s) { state = s; }
    ~FillStation();
};
