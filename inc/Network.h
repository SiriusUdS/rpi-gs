#pragma once
#include "Screen.h"

class NetworkPage: public Screen{

    public:
    
    NetworkPage();

    virtual int tick_interval_ms() const override { return 200; }
    virtual bool tick() override { return true; }

    void draw(Panel*p, bool focused);

    ~NetworkPage();
};