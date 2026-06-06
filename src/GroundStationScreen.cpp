#include "GroundStationScreen.h"
#include "system.h"
#include "sirius-headers-common/GSControl/GSControlState.h"

static const char* gs_button_names[GS_CONTROL_BUTTON_AMOUNT] = {
    "ALLOW_FILL",
    "ARM_VALVE",
    "ARM_IGNITER",
    "ALLOW_DUMP",
    "EMERGENCY_STOP",
    "FIRE_IGNITER",
    "VALVE_START",
    "UNSAFE_KEY"
};

GroundStationScreen::GroundStationScreen()
    : Screen("Ground Station") {
}

GroundStationScreen::~GroundStationScreen() {
}

void GroundStationScreen::draw(Panel* p, bool focused) {
    WINDOW* parent = p->inner();
    werase(parent);

    int avail_h = getmaxy(parent);
    int avail_w = getmaxx(parent);

    // Split layout: 45% left for Status & Buttons, 55% right for logs
    int sub_h = avail_h - 2;
    int status_w = (avail_w - 6) * 45 / 100;
    int log_w = avail_w - 6 - status_w;

    WINDOW* statusW = derwin(parent, sub_h, status_w, 1, 2);
    WINDOW* logW = derwin(parent, sub_h, log_w, 1, 2 + status_w + 2);

    if (!statusW || !logW) {
        if (statusW) delwin(statusW);
        if (logW) delwin(logW);
        return;
    }

    werase(statusW);
    werase(logW);

    // Draw borders with titles
    draw_box(statusW, "GS STATUS & INPUTS", focused);
    draw_box(logW, "ACTIVITY LOG (GS.log())", false);

    auto& gs = SiriusSystem::getInstance().getGroundStation();

    // ── Left Column: Status Display ──────────────────────────────────────────
    int y = 2;
    
    // 1. Error Flagged
    mvwprintw(statusW, y, 2, "Error Flag: ");
    if (gs.getErrorFlagged()) {
        wattron(statusW, COLOR_PAIR(CP_FAIL) | A_BOLD | A_BLINK);
        mvwprintw(statusW, y, 15, "[ FLAGGED ]");
        wattroff(statusW, COLOR_PAIR(CP_FAIL) | A_BOLD | A_BLINK);
    } else {
        wattron(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
        mvwprintw(statusW, y, 15, "[ NORMAL ]");
        wattroff(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
    }
    y += 2;

    // 2. System Request State
    mvwprintw(statusW, y, 2, "Req State:  ");
    uint8_t req_state = gs.getSystemRequestState();
    std::string req_state_str = "UNKNOWN";
    if (req_state == GS_CONTROL_STATE_INIT) req_state_str = "INIT (0x00)";
    else if (req_state == GS_CONTROL_STATE_SAFE) req_state_str = "SAFE (0x01)";
    else if (req_state == GS_CONTROL_STATE_UNSAFE) req_state_str = "UNSAFE (0x02)";
    else if (req_state == GS_CONTROL_STATE_ABORT) req_state_str = "ABORT (0x03)";
    
    wattron(statusW, COLOR_PAIR(CP_LABEL) | A_BOLD);
    mvwprintw(statusW, y, 15, "%s", req_state_str.c_str());
    wattroff(statusW, COLOR_PAIR(CP_LABEL) | A_BOLD);
    y += 2;

    // 3. Device Connections
    mvwprintw(statusW, y, 2, "Device Connections:");
    y += 1;
    
    mvwprintw(statusW, y, 4, "- Engine Board: ");
    if (gs.isEngineConnected()) {
        wattron(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
        mvwprintw(statusW, y, 20, "CONNECTED");
        wattroff(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
    } else {
        wattron(statusW, COLOR_PAIR(CP_FAIL));
        mvwprintw(statusW, y, 20, "DISCONNECTED");
        wattroff(statusW, COLOR_PAIR(CP_FAIL));
    }
    y += 1;

    mvwprintw(statusW, y, 4, "- Fill Station: ");
    if (gs.isFillConnected()) {
        wattron(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
        mvwprintw(statusW, y, 20, "CONNECTED");
        wattroff(statusW, COLOR_PAIR(CP_GRAPH) | A_BOLD);
    } else {
        wattron(statusW, COLOR_PAIR(CP_FAIL));
        mvwprintw(statusW, y, 20, "DISCONNECTED");
        wattroff(statusW, COLOR_PAIR(CP_FAIL));
    }
    y += 2;

    // 4. Hardware GPIO Buttons State
    mvwprintw(statusW, y, 2, "GS Control Buttons (GPIO):");
    y += 1;

    int col1_x = 4;
    int col2_x = status_w / 2 + 1;
    const ButtonState* buttons = gs.getButtons();

    for (int i = 0; i < 4; i++) {
        // Column 1 (Buttons 0 to 3)
        mvwprintw(statusW, y + i, col1_x, "%-11.11s: ", gs_button_names[i]);
        if (buttons[i].is_pressed.load()) {
            wattron(statusW, COLOR_PAIR(CP_LABEL) | A_BOLD);
            wprintw(statusW, "[ ON ]");
            wattroff(statusW, COLOR_PAIR(CP_LABEL) | A_BOLD);
        } else {
            wattron(statusW, A_DIM);
            wprintw(statusW, "[OFF]");
            wattroff(statusW, A_DIM);
        }

        // Column 2 (Buttons 4 to 7)
        int idx = i + 4;
        mvwprintw(statusW, y + i, col2_x, "%-11.11s: ", gs_button_names[idx]);
        if (buttons[idx].is_pressed.load()) {
            // Critical warning (E-Stop) in red/fail color, others in label color
            int cp = (idx == 4) ? CP_FAIL : CP_LABEL;
            wattron(statusW, COLOR_PAIR(cp) | A_BOLD);
            wprintw(statusW, "[ ON ]");
            wattroff(statusW, COLOR_PAIR(cp) | A_BOLD);
        } else {
            wattron(statusW, A_DIM);
            wprintw(statusW, "[OFF]");
            wattroff(statusW, A_DIM);
        }
    }


    // ── Right Column: Scrolling Logs ─────────────────────────────────────────
    const auto& logs = gs.getLogs();
    int log_display_h = sub_h - 2;
    int start_idx = std::max(0, (int)logs.size() - log_display_h);
    for (int i = 0; i < log_display_h && (start_idx + i) < (int)logs.size(); ++i) {
        std::string line = logs[start_idx + i];
        if ((int)line.size() > log_w - 4) {
            line = line.substr(0, log_w - 7) + "...";
        }
        mvwprintw(logW, 1 + i, 2, "%s", line.c_str());
    }


    // ── Bottom Legend ────────────────────────────────────────────────────────
    wattron(parent, COLOR_PAIR(CP_LABEL));
    std::string legend = " Ground Station Status Monitor (Viewer Mode) ";
    int legend_x = (avail_w - (int)legend.size()) / 2;
    if (legend_x > 0) {
        mvwprintw(parent, avail_h - 1, legend_x, "%s", legend.c_str());
    }
    wattroff(parent, COLOR_PAIR(CP_LABEL));

    wnoutrefresh(parent);
    wnoutrefresh(statusW);
    wnoutrefresh(logW);

    // ── Popup overlays (exclusive, priority: ABORT > DANGER > ALLOW_FILL) ────
    if (req_state == GS_CONTROL_STATE_ABORT) {
        int popup_h = 8;
        int popup_w = 40;
        int popup_y = (avail_h - popup_h) / 2;
        int popup_x = (avail_w - popup_w) / 2;

        WINDOW* popupW = derwin(parent, popup_h, popup_w, popup_y, popup_x);
        if (popupW) {
            werase(popupW);
            wbkgd(popupW, COLOR_PAIR(CP_DANGER));
            wattron(popupW, COLOR_PAIR(CP_DANGER) | A_BOLD | A_BLINK);
            box(popupW, 0, 0);

            std::string title = " EMERGENCY ABORT ";
            mvwprintw(popupW, 0, (popup_w - (int)title.size()) / 2, "%s", title.c_str());

            std::string msg1 = "SYSTEM ABORTED!";
            std::string msg2 = "EMERGENCY BUTTON ACTIVE";
            std::string msg3 = "ALL OPERATIONS HALTED";
            std::string msg4 = "IMMEDIATE RESET REQUIRED";

            mvwprintw(popupW, 2, (popup_w - (int)msg1.size()) / 2, "%s", msg1.c_str());
            mvwprintw(popupW, 3, (popup_w - (int)msg2.size()) / 2, "%s", msg2.c_str());
            mvwprintw(popupW, 4, (popup_w - (int)msg3.size()) / 2, "%s", msg3.c_str());
            mvwprintw(popupW, 5, (popup_w - (int)msg4.size()) / 2, "%s", msg4.c_str());
            wattroff(popupW, COLOR_PAIR(CP_DANGER) | A_BOLD | A_BLINK);

            wnoutrefresh(popupW);
            delwin(popupW);
        }
    } else if (req_state == GS_CONTROL_STATE_UNSAFE &&
               buttons[1].is_pressed.load() &&
               buttons[2].is_pressed.load()) {
        int popup_h = 8;
        int popup_w = 40;
        int popup_y = (avail_h - popup_h) / 2;
        int popup_x = (avail_w - popup_w) / 2;

        WINDOW* popupW = derwin(parent, popup_h, popup_w, popup_y, popup_x);
        if (popupW) {
            werase(popupW);
            wbkgd(popupW, COLOR_PAIR(CP_DANGER));
            wattron(popupW, COLOR_PAIR(CP_DANGER) | A_BOLD);
            box(popupW, 0, 0);

            std::string title = " DANGER WARNING ";
            mvwprintw(popupW, 0, (popup_w - (int)title.size()) / 2, "%s", title.c_str());

            std::string msg1 = "CRITICAL CONDITION!";
            std::string msg2 = "ARM VALVES & ARM IGNITER";
            std::string msg3 = "ARE BOTH ON UNDER";
            std::string msg4 = "UNSAFE STATE!";

            mvwprintw(popupW, 2, (popup_w - (int)msg1.size()) / 2, "%s", msg1.c_str());
            mvwprintw(popupW, 3, (popup_w - (int)msg2.size()) / 2, "%s", msg2.c_str());
            mvwprintw(popupW, 4, (popup_w - (int)msg3.size()) / 2, "%s", msg3.c_str());
            mvwprintw(popupW, 5, (popup_w - (int)msg4.size()) / 2, "%s", msg4.c_str());
            wattroff(popupW, COLOR_PAIR(CP_DANGER) | A_BOLD);

            wnoutrefresh(popupW);
            delwin(popupW);
        }
    } else if (buttons[0].is_pressed.load()) {
        int popup_h = 7;
        int popup_w = 40;
        int popup_y = (avail_h - popup_h) / 2;
        int popup_x = (avail_w - popup_w) / 2;

        WINDOW* popupW = derwin(parent, popup_h, popup_w, popup_y, popup_x);
        if (popupW) {
            werase(popupW);
            wbkgd(popupW, COLOR_PAIR(CP_WARNING));
            wattron(popupW, COLOR_PAIR(CP_WARNING) | A_BOLD);
            box(popupW, 0, 0);

            std::string title = " WARNING ";
            mvwprintw(popupW, 0, (popup_w - (int)title.size()) / 2, "%s", title.c_str());

            std::string msg1 = "ALLOW FILL ACTIVE!";
            std::string msg2 = "FILL STATION OPERATION";
            std::string msg3 = "IS PERMITTED";

            mvwprintw(popupW, 2, (popup_w - (int)msg1.size()) / 2, "%s", msg1.c_str());
            mvwprintw(popupW, 3, (popup_w - (int)msg2.size()) / 2, "%s", msg2.c_str());
            mvwprintw(popupW, 4, (popup_w - (int)msg3.size()) / 2, "%s", msg3.c_str());
            wattroff(popupW, COLOR_PAIR(CP_WARNING) | A_BOLD);

            wnoutrefresh(popupW);
            delwin(popupW);
        }
    }

    delwin(logW);
    delwin(statusW);
}

bool GroundStationScreen::tick() {
    return true; // Force redraw to show heartbeat/log/button updates
}

bool GroundStationScreen::handle_key(int key) {
    if (key == 27 || key == KEY_LEFT) { // ESC or LEFT
        return false; // hand focus back to menu
    }
    return true; // swallow other keys
}
