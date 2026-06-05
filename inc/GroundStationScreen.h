#pragma once

#include "Screen.h"

class GroundStationScreen : public Screen {
public:
    GroundStationScreen();
    virtual ~GroundStationScreen();

    virtual int tick_interval_ms() const override { return 100; }
    virtual void draw(Panel* p, bool focused) override;
    virtual bool tick() override;
    virtual bool handle_key(int key) override;

};
