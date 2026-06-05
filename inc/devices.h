#pragma once
#include "Screen.h"
#include "FillStation.h"
#include "Engine.h"


class DevicesPages: public Screen{
    private:
    int focused_sub; // 0 = fill, 1 = engine
    public:


    DevicesPages();
    virtual int  tick_interval_ms() const { return 100; }  // 0 = static screen
    void draw(Panel*p, bool focused);
    virtual bool tick();
    virtual bool handle_key(int key) override;
    ~DevicesPages();
};