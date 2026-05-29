#include "Welcome.h"

void Welcome::drawList(WINDOW *w, int offx, int offy)
{
    for(int i =0; i < list.size(); i++){
        Label(w, offy+i, offx, list[i], CP_LABEL, A_BOLD).draw();
    }
}

Welcome::Welcome() : Screen("WELCOME PAGE")
{

}


void Welcome::draw(Panel *p, bool focused)
{
    WINDOW* w = p->inner();
    werase(w);

    Label(w, 0,0, "This TUI allows you to navigate through the main component of the system.", CP_LABEL, A_BOLD).draw();
    Label(w, 3,0, "Operation check list: ", CP_LABEL, A_BOLD).draw();
    drawList(w, 0,4);
    wnoutrefresh(w);
}

Welcome::~Welcome()
{
}
