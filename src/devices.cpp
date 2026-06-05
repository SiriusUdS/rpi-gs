#include "devices.h"
#include "system.h"
#include "sirius-headers-common/Engine/EngineState.h"
#include "sirius-headers-common/FillingStation/FillingStationState.h"
#include "sirius-headers-common/Ethernet/UDPFrame.h"
#include "sirius-headers-common/Telecommunication/BoardCommandV2.h"
#include "sirius-headers-common/Telecommunication/PacketHeaderVariable.h"
#include <cstring>

DevicesPages::DevicesPages()
    : Screen("Devices"), focused_sub(0)
{
}

WINDOW* create_sub(WINDOW* parent, int sub_h, int sub_w, int y, int x) {
    if (sub_h <= 0 || sub_w <= 0) {
        wnoutrefresh(parent);
        return nullptr;
    }
    WINDOW* subW = derwin(parent, sub_h, sub_w, y, x);
    box(subW, 0, 0);
    return subW;
}

void DevicesPages::draw(Panel* p, bool focused)
{
    WINDOW* parent = p->inner();
    werase(parent);

    int avail_h = getmaxy(parent);
    int avail_w = getmaxx(parent);

    // Calculate dimensions to split into 2 equal panels
    int sub_h = avail_h - 2;
    int sub_w = (avail_w - 6) / 2;

    WINDOW* fillW = create_sub(parent, sub_h, sub_w, 1, 2);
    WINDOW* engW = create_sub(parent, sub_h, sub_w, 1, 2 + sub_w + 2);

    if (fillW == nullptr || engW == nullptr) {
        if (fillW) delwin(fillW);
        if (engW) delwin(engW);
        return;
    }

    // Pass whether individual sub-panel is focused and connected (received UDP packet in last 5 seconds / 50 ticks)
    auto& gs = SiriusSystem::getInstance().getGroundStation();
    gs.getFillStation().draw(fillW, focused && (focused_sub == 0), gs.isFillConnected());
    gs.getEngine().draw(engW, focused && (focused_sub == 1), gs.isEngineConnected());

    // Draw control instructions/legend in the middle at the bottom
    wattron(parent, COLOR_PAIR(CP_LABEL));
    std::string legend = " Device Status Monitor (Viewer Mode) ";
    int legend_x = (avail_w - (int)legend.size()) / 2;
    if (legend_x > 0) {
        mvwprintw(parent, avail_h - 1, legend_x, "%s", legend.c_str());
    }
    wattroff(parent, COLOR_PAIR(CP_LABEL));

    wnoutrefresh(parent);
    wnoutrefresh(fillW);
    wnoutrefresh(engW);
    delwin(engW);
    delwin(fillW);
}



bool DevicesPages::tick()
{
    return false;
}

bool DevicesPages::handle_key(int key)
{
    if (key == 27 || key == KEY_LEFT) { // ESC or LEFT
        return false; // hand focus back
    }
    if (key == '\t' || key == KEY_RIGHT) {
        focused_sub = 1 - focused_sub;
        return true;
    }
    return true;
}

DevicesPages::~DevicesPages()
{
}
