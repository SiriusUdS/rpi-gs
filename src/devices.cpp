#include "devices.h"

DevicesPages::DevicesPages(): Screen("Devices")
{
}

WINDOW* create_sub(WINDOW* parent, int avail_h, int avail_w, int y, int x){
    int sub_h = avail_h/2 +10;
    int sub_w = avail_w/ 3 -4;
    int sub_y = y;
    int sub_x = x;

    if (sub_h <= 0 || sub_w <= 0) {
        wnoutrefresh(parent);
        return nullptr;
    }

    WINDOW* fillW = derwin(parent, sub_h, sub_w, sub_y, sub_x);
    box(fillW, 0, 0);
    return fillW;
}

void DevicesPages::draw(Panel* p, bool focused)
{
    WINDOW* parent = p->inner();
    werase(parent);

    // derwin(parent, rows, cols, y_offset, x_offset)
    // Use p->_rows/_cols for the panel's dimensions, minus border inset
    int avail_h = getmaxy(parent);
    int avail_w = getmaxx(parent);

    WINDOW* fillW = create_sub(parent, avail_h, avail_w, 2, 2);
    WINDOW* engW = create_sub(parent, avail_h, avail_w, 2, 50);

    if (fillW== nullptr || engW == nullptr)return;

    fill.draw(fillW);          // your fill widget draws into fillW
    engine.draw(engW);

    wnoutrefresh(parent);      // stage parent
    wnoutrefresh(fillW);
    wnoutrefresh(engW);
    delwin(engW);
    delwin(fillW);             // derwin'd windows must be deleted when done
    // doupdate() is called by draw_all() in main — do NOT call it here
}

bool DevicesPages::tick()
{

    return false;
}

DevicesPages::~DevicesPages()
{

}
