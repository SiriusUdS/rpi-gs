#include "Engine.h"
#include "sirius-headers-common/Engine/EngineState.h"

Engine::Engine(): state(ENGINE_STATE_INIT)
{
}

static void drawNode(WINDOW* w, int y, int x, int width, const std::string& label, bool active, bool focused) {
    int cp = CP_NORMAL;
    int attr = A_DIM;

    if (active) {
        attr = A_BOLD;
        if (label == "ABORT" || label == "ERROR") {
            cp = CP_FAIL;
        } else if (label == "TEST") {
            cp = CP_LABEL;
        } else {
            cp = focused ? CP_SELECTED : CP_TITLE;
        }
    } else {
        attr = focused ? A_NORMAL : A_DIM;
    }
    
    wattron(w, COLOR_PAIR(cp) | attr);
    // Top border
    mvwaddch(w, y, x, ACS_ULCORNER);
    for (int i = 1; i < width - 1; ++i) mvwaddch(w, y, x + i, ACS_HLINE);
    mvwaddch(w, y, x + width - 1, ACS_URCORNER);
    
    // Middle row (text)
    mvwaddch(w, y + 1, x, ACS_VLINE);
    int text_width = width - 4;
    mvwprintw(w, y + 1, x + 2, "%-*.*s", text_width, text_width, label.c_str());
    mvwaddch(w, y + 1, x + width - 1, ACS_VLINE);
    
    // Bottom border
    mvwaddch(w, y + 2, x, ACS_LLCORNER);
    for (int i = 1; i < width - 1; ++i) mvwaddch(w, y + 2, x + i, ACS_HLINE);
    mvwaddch(w, y + 2, x + width - 1, ACS_LRCORNER);
    wattroff(w, COLOR_PAIR(cp) | attr);
}

static void drawConnectionMissingPopup(WINDOW* w) {
    int cols = getmaxx(w);
    int rows = getmaxy(w);
    
    int box_w = 24;
    int box_h = 5;
    int x = (cols - box_w) / 2;
    int y = (rows - box_h) / 2;
    
    wattron(w, COLOR_PAIR(CP_FAIL) | A_BOLD);
    
    // Draw box border manually
    mvwaddch(w, y, x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; ++i) mvwaddch(w, y, x + i, ACS_HLINE);
    mvwaddch(w, y, x + box_w - 1, ACS_URCORNER);
    
    for (int r = 1; r < box_h - 1; ++r) {
        mvwaddch(w, y + r, x, ACS_VLINE);
        mvwprintw(w, y + r, x + 1, "%*s", box_w - 2, "");
        mvwaddch(w, y + r, x + box_w - 1, ACS_VLINE);
    }
    
    mvwaddch(w, y + box_h - 1, x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; ++i) mvwaddch(w, y + box_h - 1, x + i, ACS_HLINE);
    mvwaddch(w, y + box_h - 1, x + box_w - 1, ACS_LRCORNER);
    
    // Draw text inside
    std::string line1 = "ATTENTION";
    std::string line2 = "Connection Missing";
    mvwprintw(w, y + 1, x + (box_w - (int)line1.size()) / 2, "%s", line1.c_str());
    
    wattroff(w, A_BOLD);
    wattron(w, COLOR_PAIR(CP_FAIL));
    mvwprintw(w, y + 2, x + (box_w - (int)line2.size()) / 2, "%s", line2.c_str());
    wattroff(w, COLOR_PAIR(CP_FAIL));
}

void Engine::draw(WINDOW *w, bool focused, bool connected)
{
    // Draw title
    int cols = getmaxx(w);
    wattron(w, COLOR_PAIR(focused ? CP_BORDER_F : CP_BORDER) | A_BOLD);
    mvwprintw(w, 0, (cols - 8) / 2, " ENGINE ");
    wattroff(w, COLOR_PAIR(focused ? CP_BORDER_F : CP_BORDER) | A_BOLD);

    int cx = (cols - 12) / 2; // Center of main flow
    int left_x = cx - 11;     // Left node x
    int right_x = cx + 13;    // Right node x

    // Draw central flow states
    drawNode(w, 2, cx, 12, "INIT", state == ENGINE_STATE_INIT, focused);
    
    // Arrow INIT -> SAFE
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 5, cx + 5, ACS_DARROW);
    wattroff(w, COLOR_PAIR(CP_BORDER));

    drawNode(w, 6, cx, 12, "SAFE", state == ENGINE_STATE_SAFE, focused);

    // Arrow SAFE -> TEST
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 7, cx + 12, ACS_HLINE);
    mvwaddch(w, 7, cx + 13, ACS_RARROW);
    wattroff(w, COLOR_PAIR(CP_BORDER));
    drawNode(w, 6, right_x, 10, "TEST", state == ENGINE_STATE_TEST, focused);

    // Arrow SAFE -> UNSAFE
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 9, cx + 5, ACS_DARROW);
    wattroff(w, COLOR_PAIR(CP_BORDER));

    drawNode(w, 10, cx, 12, "UNSAFE", state == ENGINE_STATE_UNSAFE, focused);

    // Arrow UNSAFE -> ABORT
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 11, cx - 1, ACS_LARROW);
    mvwaddch(w, 11, cx - 2, ACS_HLINE);
    wattroff(w, COLOR_PAIR(CP_BORDER));
    drawNode(w, 10, left_x, 10, "ABORT", state == ENGINE_STATE_ABORT, focused);

    // Arrow UNSAFE -> IGNITION
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 13, cx + 5, ACS_DARROW);
    wattroff(w, COLOR_PAIR(CP_BORDER));

    drawNode(w, 14, cx, 12, "IGNITION", state == ENGINE_STATE_IGNITION, focused);

    // Arrow IGNITION -> ERROR
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 15, cx - 1, ACS_LARROW);
    mvwaddch(w, 15, cx - 2, ACS_HLINE);
    wattroff(w, COLOR_PAIR(CP_BORDER));
    drawNode(w, 14, left_x, 10, "ERROR", state == ENGINE_STATE_ERROR, focused);

    // Arrow IGNITION -> FIRE
    wattron(w, COLOR_PAIR(CP_BORDER));
    mvwaddch(w, 17, cx + 5, ACS_DARROW);
    wattroff(w, COLOR_PAIR(CP_BORDER));

    drawNode(w, 18, cx, 12, "FIRE", state == ENGINE_STATE_FIRE, focused);

    if (!connected) {
        drawConnectionMissingPopup(w);
    }
}

Engine::~Engine()
{
}
