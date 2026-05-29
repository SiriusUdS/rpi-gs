#include "FillStation.h"


FillStation::FillStation(): state(0)
{
}

void FillStation::draw(WINDOW *w)
{
    Label(w, 0,1, "FILLSTATION", CP_TITLE, A_BOLD).draw();

    Label(w, 2, 2, "ID : 4", CP_LABEL, A_BOLD).draw();
}

FillStation::~FillStation()
{
}
