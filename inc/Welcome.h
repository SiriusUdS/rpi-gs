#pragma once
#include "Screen.h"
#include <vector>

class Welcome: public Screen
{
private:
    /* data */
    std::vector<std::string> list = {
        "- Install the antennas and power them on",
        "- Start the GS",
        "- Go to the tab: Network and validate the connection",
        "- Engage the procedure"
    };
    void drawList(WINDOW* w, int offx, int offy);
public:
    Welcome(/* args */);
    void draw(Panel* p, bool focused);
    ~Welcome();
};
