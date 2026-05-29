#include "Welcome.h"
#include "devices.h"
#include "Network.h"
#include <vector>
#include <memory>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include "ttyScreen.h"

using Clock = std::chrono::steady_clock;
using Ms    = std::chrono::milliseconds;

// ═════════════════════════════════════════════════════════════════════════════
//  Layout helpers
// ═════════════════════════════════════════════════════════════════════════════
enum class Focus { NAV, CONTENT };

std::pair<Panel*, Panel*> make_panels() {
    int h = LINES - 2, lw = COLS / 3;
    return { new Panel(h, lw,        1, 0,  "Navigation"),
             new Panel(h, COLS - lw, 1, lw, "Content")    };
}

// ═════════════════════════════════════════════════════════════════════════════
//  Main
// ═════════════════════════════════════════════════════════════════════════════
int main() {
    TUI::init();
    srand(42);

    Welcome wel;

    // ── Register screens ─────────────────────────────────────────────────────
    std::vector<std::unique_ptr<Screen>> screens;
    screens.push_back(std::make_unique<NetworkPage>());
    screens.push_back(std::make_unique<DevicesPages>());
    screens.push_back(std::make_unique<HandoffScreen>("Shell", "/bin/sh"));

    // ── Build nav from screen titles ─────────────────────────────────────────
    std::vector<MenuItem> nav_items;
    for (auto& s : screens)
        nav_items.push_back({s->getTitle(), nullptr, {}, false});
    nav_items.push_back({"", nullptr, {}, true});
    nav_items.push_back({
        "Settings", nullptr, {
            {"Theme",    []{ Dialog::show("Theme",    "Theme changed!",  {"OK"}); }},
            {"Language", []{ Dialog::show("Language", "Language saved.", {"OK"}); }},
            {"", nullptr, {}, true},
            {"Advanced", nullptr, {
                {"Reset all", []{ Dialog::show("Reset",  "All settings reset.", {"OK"}); }},
                {"Export",    []{ Dialog::show("Export", "Config exported.",    {"OK"}); }},
            }, false},
        }, false
    });

    // ── App state ─────────────────────────────────────────────────────────────
    TitleBar  title("#  Sirius Dashboard  #");
    StatusBar status;
    auto [left, right] = make_panels();
    Menu  nav(left->inner(), nav_items);
    Focus focus      = Focus::NAV;
    int   cur_screen = -1;   // -1 = welcome screen, no active Screen object

    // Helper: is a real screen active?
    auto has_screen = [&]() { return cur_screen >= 0; };

    auto update_hint = [&]() {
        status.set(focus == Focus::NAV
            ? "UP/DOWN navigate   RIGHT/ENTER open screen"
            : "LEFT/ESC back to menu   Q quit");
    };

    auto draw_all = [&]() {
        title.draw();
        status.draw();
        wnoutrefresh(stdscr);
        bool nf = (focus == Focus::NAV), cf = (focus == Focus::CONTENT);
        left->set_focused(nf);  left->draw_border();  nav.draw(nf);
        right->set_focused(cf); right->draw_border();
        if (has_screen())
            screens[cur_screen]->draw(right, cf);
        else
            wel.draw(right, cf);
        doupdate();
    };

    // Only call tick/timeout on the active Screen object, never on welcome
    auto apply_timeout = [&]() {
        int ms = has_screen() ? screens[cur_screen]->tick_interval_ms() : 0;
        wtimeout(left->win(), ms > 0 ? ms : -1);
    };

    update_hint();
    apply_timeout();
    draw_all();

    // ── Event loop ────────────────────────────────────────────────────────────
    auto last_tick = Clock::now();
    int key;
    while ((key = wgetch(left->win())) != ERR || true) {
        bool needs_redraw = false;

        if (key == ERR) {
            // Timeout — only tick if a real screen is active
            if (has_screen()) {
                auto now     = Clock::now();
                auto elapsed = std::chrono::duration_cast<Ms>(now - last_tick).count();
                int  interval = screens[cur_screen]->tick_interval_ms();
                if (interval > 0 && elapsed >= interval) {
                    if (screens[cur_screen]->tick()) needs_redraw = true;
                    last_tick = now;
                }
            }
        } else {
            needs_redraw = true;

            if (focus == Focus::NAV) {
                std::function<void()> action;
                int result = nav.handle_key(key, action);

                if (action) {
                    action();
                } else if (result == 1) {
                    int sel = nav.selected;
                    if (sel < (int)screens.size()) {
                        // Leave current screen if one was active
                        if (has_screen()) screens[cur_screen]->on_leave();
                        cur_screen = sel;
                        screens[cur_screen]->on_enter();
                        focus = Focus::CONTENT;
                        apply_timeout();
                    }
                } else if (result == 2) {
                    break;
                }
            } else {
                // Content focus — delegate to active screen
                if (has_screen()) {
                    if (key == 'q' || !screens[cur_screen]->handle_key(key)) {
                        screens[cur_screen]->on_leave();
                        focus = Focus::NAV;
                        cur_screen = -1;   // back to welcome on nav return
                        apply_timeout();
                    }
                } else {
                    // Welcome has no handle_key — any key returns to nav
                    if (key == KEY_LEFT || key == 27 || key == 'q')
                        focus = Focus::NAV;
                }
            }
        }

        if (needs_redraw) {
            update_hint();
            draw_all();
        }
    }

    delete left;
    delete right;
    TUI::shutdown();
    return 0;
}