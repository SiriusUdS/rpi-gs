#pragma once
#include "GroundStation.h"
#include "GpioReader.h"

class SiriusSystem{
    private:
    SiriusSystem();
    ~SiriusSystem();
    GpioReader gpio_reader;
    GroundStation ground_station;
    public:
        const ServerStatus getServerStatus();
        GroundStation& getGroundStation() { return ground_station; }
        GpioReader& getGpioReader() { return gpio_reader; }
        static SiriusSystem& getInstance();
        
};
