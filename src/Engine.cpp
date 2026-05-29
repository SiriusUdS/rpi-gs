#include "Engine.h"


Engine::Engine(): state(0)
{
}

void Engine::draw(WINDOW *w)
{
    Label(w, 0,1, "ENGINE", CP_TITLE, A_BOLD).draw();
}

Engine::~Engine()
{
}
