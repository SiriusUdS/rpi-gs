#pragma once
#include "server.h"

class SiriusSystem{
    private:
    SiriusSystem();
    ~SiriusSystem();
    UdpServer server;
    public:
        const ServerStatus getServerStatus();
        static SiriusSystem& getInstance();
        
};
