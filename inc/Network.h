#pragma once
#include "Screen.h"

class NetworkPage: public Screen{

    public:
    
    NetworkPage();


    void draw(Panel*p, bool focused);

    ~NetworkPage();
};