#include "system.h"

SiriusSystem::SiriusSystem()
    : ground_station(gpio_reader)
{
    gpio_reader.start();
}

const ServerStatus SiriusSystem::getServerStatus()
{
    return ground_station.getServer().getStatus();
}

SiriusSystem &SiriusSystem::getInstance()
{
    // Static local variable initialization is thread-safe in C++11 and later.
    // It is created exactly once, the first time this function is called.
    static SiriusSystem instance;
    return instance;
}

SiriusSystem::~SiriusSystem()
{
    gpio_reader.stop();
}
