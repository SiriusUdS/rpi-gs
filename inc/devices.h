#pragma once
#include "Screen.h"
#include "FillStation.h"
#include "Engine.h"


class DevicesPages: public Screen{
    private:
    FillStation fill;
    Engine engine;
    public:


    DevicesPages();
    virtual int  tick_interval_ms() const { return 100; }  // 0 = static screen
    void draw(Panel*p, bool focused);
    virtual bool tick();
    ~DevicesPages();
};