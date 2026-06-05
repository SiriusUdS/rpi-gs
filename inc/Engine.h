#pragma once
#include "tui.h"
#include <stdint.h>


#include <atomic>

class Engine{
    private :
    std::atomic<uint8_t> state;
    public:

    Engine();
    void draw(WINDOW* w, bool focused = false, bool connected = true);
    uint8_t getState() const { return state; }
    void setState(uint8_t s) { state = s; }
    ~Engine();
};
