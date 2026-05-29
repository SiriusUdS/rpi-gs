#include "system.h"

SiriusSystem::SiriusSystem()
{
}

const ServerStatus SiriusSystem::getServerStatus()
{
    return server.getStatus();
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
}
