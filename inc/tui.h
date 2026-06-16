#pragma once
#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>


/* ex of separator and submenu
{ "",           nullptr, {}, true  },
        {
            "Settings", nullptr,
            {
                { "Theme",    []{ Dialog::show("Theme",    "Theme changed!",  {"OK"}); } },
                { "Language", []{ Dialog::show("Language", "Language saved.", {"OK"}); } },
                { "",         nullptr, {}, true },
                {
                    "Advanced", nullptr,
                    {
                        { "Reset all", []{ Dialog::show("Reset", "All settings reset.", {"OK"}); } },
                        { "Export",    []{ Dialog::show("Export","Config exported.",    {"OK"}); } },
                    },
                    false
                },
            },
            false
        },
*/


enum ColorPair {
    CP_NORMAL   = 1,   // white on black
    CP_TITLE    = 2,   // black on cyan   (title bar / panel titles)
    CP_SELECTED = 3,   // black on white  (highlighted row / focused button)
    CP_BORDER   = 4,   // cyan on black   (box borders)
    CP_BORDER_F = 9,   // bright white on black  (focused panel border)
    CP_DIALOG   = 5,   // white on blue   (dialog bg)
    CP_STATUS   = 6,   // black on yellow (status bar)
    CP_GRAPH    = 7,   // green on black  (graph bars)
    CP_LABEL    = 8,   // yellow on black (labels / keys)
    CP_BTN      = 10,  // black on cyan   (button normal)
    CP_BTN_F    = 11,  // black on white  (button focused)
    CP_BTN_DIS  = 12,  // black on darkgray (button disabled) — fall back to normal
    CP_FAIL     = 13,
    CP_DANGER   = 14,
    CP_WARNING  = 15,
};

struct TUI {
    static void init() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        start_color();
        use_default_colors();

        init_pair(CP_NORMAL,   COLOR_BLACK,   COLOR_WHITE);
        init_pair(CP_TITLE,    COLOR_WHITE,   COLOR_BLUE);
        init_pair(CP_SELECTED, COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_BORDER,   COLOR_BLUE,    COLOR_WHITE);
        init_pair(CP_BORDER_F, COLOR_BLACK,   COLOR_WHITE);
        init_pair(CP_DIALOG,   COLOR_BLACK,   COLOR_CYAN);
        init_pair(CP_STATUS,   COLOR_WHITE,   COLOR_BLUE);
        init_pair(CP_GRAPH,    COLOR_GREEN,   COLOR_WHITE);
        init_pair(CP_LABEL,    COLOR_BLUE,    COLOR_WHITE);
        init_pair(CP_BTN,      COLOR_BLACK,   COLOR_CYAN);
        init_pair(CP_BTN_F,    COLOR_WHITE,   COLOR_BLUE);
        init_pair(CP_BTN_DIS,  COLOR_BLACK,   COLOR_WHITE);
        init_pair(CP_FAIL,     COLOR_RED,     COLOR_WHITE);
        init_pair(CP_DANGER,   COLOR_WHITE,   COLOR_RED);
        init_pair(CP_WARNING,  COLOR_BLACK,   COLOR_YELLOW);

        bkgd(COLOR_PAIR(CP_NORMAL));
    }
    static void shutdown() { endwin(); }
};

// ─────────────────────────────────────────────
//  Helper: draw a box; focused = brighter border
// ─────────────────────────────────────────────
inline void draw_box(WINDOW* w, const std::string& title = "", bool focused = false) {
    int cp = focused ? CP_BORDER_F : CP_BORDER;
    int at = focused ? (COLOR_PAIR(cp) | A_BOLD) : COLOR_PAIR(cp);
    wattron(w, at);
    box(w, 0, 0);
    wattroff(w, at);
    if (!title.empty()) {
        int cols = getmaxx(w);
        int x = std::max(1, (cols - (int)title.size() - 2) / 2);
        wattron(w, COLOR_PAIR(CP_TITLE) | A_BOLD);
        mvwprintw(w, 0, x, " %s ", title.c_str());
        wattroff(w, COLOR_PAIR(CP_TITLE) | A_BOLD);
    }
}


struct TitleBar {
    std::string text;
    explicit TitleBar(const std::string& t) : text(t) {}
    void draw() const {
        attron(COLOR_PAIR(CP_TITLE) | A_BOLD);
        mvhline(0, 0, ' ', COLS);
        mvprintw(0, (COLS - (int)text.size()) / 2, "%s", text.c_str());
        attroff(COLOR_PAIR(CP_TITLE) | A_BOLD);
    }
};


struct StatusBar {
    std::string text;
    explicit StatusBar(const std::string& t = "") : text(t) {}
    void set(const std::string& t) { text = t; }
    void draw() const {
        attron(COLOR_PAIR(CP_STATUS));
        mvhline(LINES - 1, 0, ' ', COLS);
        mvprintw(LINES - 1, 1, "%s", text.c_str());
        attroff(COLOR_PAIR(CP_STATUS));
    }
};

struct Panel {
    WINDOW*     _win   = nullptr;
    WINDOW*     _inner = nullptr;
    int         _rows, _cols, _y, _x;
    std::string _title;
    bool        _focused = false;

    Panel(int rows, int cols, int y, int x, const std::string& title = "")
        : _rows(rows), _cols(cols), _y(y), _x(x), _title(title)
    {
        _win   = newwin(rows, cols, y, x);
        _inner = derwin(_win, rows - 2, cols - 2, 1, 1);
        keypad(_win, TRUE);
        keypad(_inner, TRUE);
        wbkgd(_win, COLOR_PAIR(CP_NORMAL));
        wbkgd(_inner, COLOR_PAIR(CP_NORMAL));
    }
    ~Panel() {
        if (_inner) delwin(_inner);
        if (_win)   delwin(_win);
    }
    Panel(const Panel&)            = delete;
    Panel& operator=(const Panel&) = delete;

    WINDOW* win()   const { return _win;   }
    WINDOW* inner() const { return _inner; }
    void set_focused(bool f) { _focused = f; }

    void draw_border() {
        werase(_win);
        draw_box(_win, _title, _focused);
        wnoutrefresh(_win);
    }
    // Full refresh: border then inner
    void refresh_all() { draw_border(); wnoutrefresh(_inner); }
};


struct Label {
    WINDOW*     _win;
    int         _y, _x;
    std::string _text;
    int         _color_pair;
    int         _attrs;

    Label(WINDOW* w, int y, int x,
          const std::string& text,
          int color_pair = CP_LABEL,
          int attrs = A_BOLD)
        : _win(w), _y(y), _x(x), _text(text),
          _color_pair(color_pair), _attrs(attrs) {}

    void set(const std::string& t) { _text = t; }
    void draw() const {
        wattron(_win, COLOR_PAIR(_color_pair) | _attrs);
        mvwprintw(_win, _y, _x, "%s", _text.c_str());
        wattroff(_win, COLOR_PAIR(_color_pair) | _attrs);
    }
};

//  Button — clickable widget, navigable with Tab/arrows
//
//  Usage in a screen:
//    ButtonGroup bg;
//    bg.add("Save",   []{ /* ... */ });
//    bg.add("Cancel", []{ /* ... */ });
//    bg.draw(win, y, x);          // draw all side-by-side
//    bg.handle_key(key);          // returns true while alive
struct Button {
    std::string            label;
    std::function<void()>  action;
    bool                   disabled = false;
};

struct ButtonGroup {
    std::vector<Button> buttons;
    int                 focused = 0;   // which button has focus

    void add(const std::string& label,
             std::function<void()> action = nullptr,
             bool disabled = false)
    {
        buttons.push_back({label, action, disabled});
    }

    // Draw all buttons on a single row at (y, x) in window w
    void draw(WINDOW* w, int y, int x, bool group_focused = true) const {
        int cx = x;
        for (int i = 0; i < (int)buttons.size(); ++i) {
            const auto& b = buttons[i];
            bool sel = group_focused && (i == focused);
            int  cp  = b.disabled ? CP_BTN_DIS : (sel ? CP_BTN_F : CP_BTN);
            int  at  = sel ? A_BOLD : 0;
            wattron(w, COLOR_PAIR(cp) | at);
            mvwprintw(w, y, cx, "[ %s ]", b.label.c_str());
            wattroff(w, COLOR_PAIR(cp) | at);
            cx += (int)b.label.size() + 5;  // "[ " + label + " ]" + 1 space
        }
    }

    // Returns false when the group wants to give focus back (ESC / left past start / right past end)
    // Activate = Enter/Space triggers the focused button action
    bool handle_key(int key) {
        int n = (int)buttons.size();
        if (n == 0) return false;
        if (key == KEY_RIGHT || key == '\t') {
            if (focused < n - 1) { focused++; return true; }
            return false;   // hand focus back
        }
        if (key == KEY_LEFT) {
            if (focused > 0) { focused--; return true; }
            return false;
        }
        if (key == '\n' || key == KEY_ENTER || key == ' ') {
            auto& b = buttons[focused];
            if (!b.disabled && b.action) b.action();
            return true;
        }
        return true;  // swallow other keys
    }
};


//  MenuItem & Menu
//
//  Submenu rules:
//  - RIGHT arrow / Enter opens a submenu
//  - LEFT  arrow / ESC   closes a submenu (returns to parent)
//  - The submenu is drawn as an overlay; the main frame stays intact
//    because we only refresh the submenu's own window.
struct MenuItem {
    std::string            label;
    std::function<void()>  action;
    std::vector<MenuItem>  submenu;
    bool                   separator = false;
};

// Forward declare so open_submenu can return a result
struct Menu;


// Drawn at absolute screen coordinates (sy, sx).
// Blocks until user presses ESC/LEFT (cancel) or Enter on a leaf item.
static std::function<void()> run_submenu_overlay(
    const std::vector<MenuItem>& items, int sy, int sx)
{
    // Compute width from longest label
    int w = 16;
    for (auto& it : items)
        w = std::max(w, (int)it.label.size() + 6);
    int h = (int)items.size() + 2;  // +2 for border

    // Clamp to screen
    if (sx + w > COLS)  sx = COLS  - w;
    if (sy + h > LINES) sy = LINES - h;

    WINDOW* bwin = newwin(h, w, sy, sx);
    WINDOW* iwin = derwin(bwin, h - 2, w - 2, 1, 1);
    keypad(bwin, TRUE);
    wbkgd(bwin, COLOR_PAIR(CP_NORMAL));
    wbkgd(iwin, COLOR_PAIR(CP_NORMAL));

    int sel = 0;
    // skip leading separators
    while (sel < (int)items.size() && items[sel].separator) sel++;

    std::function<void()> result = nullptr;
    bool done = false;

    auto redraw = [&]() {
        werase(bwin);
        draw_box(bwin, "", true);
        int rows = getmaxy(iwin);
        for (int i = 0; i < (int)items.size() && i < rows; ++i) {
            const auto& it = items[i];
            if (it.separator) {
                wattron(iwin, COLOR_PAIR(CP_BORDER));
                mvwhline(iwin, i, 0, ACS_HLINE, getmaxx(iwin));
                wattroff(iwin, COLOR_PAIR(CP_BORDER));
                continue;
            }
            int mw = getmaxx(iwin);
            if (i == sel) {
                wattron(iwin, COLOR_PAIR(CP_SELECTED) | A_BOLD);
                mvwprintw(iwin, i, 0, "%-*s", mw, (" " + it.label).c_str());
                if (!it.submenu.empty()) mvwaddch(iwin, i, mw - 1, '>');
                wattroff(iwin, COLOR_PAIR(CP_SELECTED) | A_BOLD);
            } else {
                wattron(iwin, COLOR_PAIR(CP_NORMAL));
                mvwprintw(iwin, i, 0, " %-*s", mw - 1, it.label.c_str());
                if (!it.submenu.empty()) mvwaddch(iwin, i, mw - 1, '>');
                wattroff(iwin, COLOR_PAIR(CP_NORMAL));
            }
        }
        wnoutrefresh(bwin);
        wnoutrefresh(iwin);
        doupdate();
    };

    redraw();

    int n = (int)items.size();
    int key;
    while (!done && (key = wgetch(bwin)) != ERR) {
        if (key == KEY_UP) {
            do { sel = (sel - 1 + n) % n; } while (items[sel].separator);
        } else if (key == KEY_DOWN) {
            do { sel = (sel + 1) % n; } while (items[sel].separator);
        } else if (key == KEY_LEFT || key == 27 /*ESC*/) {
            done = true;   // cancel
        } else if (key == KEY_RIGHT || key == '\n' || key == KEY_ENTER) {
            const auto& it = items[sel];
            if (!it.submenu.empty()) {
                // Recurse: open nested submenu to the right
                int nsy = sy + 1 + sel;
                int nsx = sx + w - 1;
                auto sub_result = run_submenu_overlay(it.submenu, nsy, nsx);
                // After sub closes, redraw ourselves so it's visible again
                if (sub_result) { result = sub_result; done = true; }
                else redraw();
            } else {
                result = it.action;
                done = true;
            }
        }
        if (!done) redraw();
    }

    // Cleanup: erase and touch parent area so caller can refresh
    werase(bwin);
    wnoutrefresh(bwin);
    delwin(iwin);
    delwin(bwin);
    // Force the area behind to repaint on next global refresh

    return result;
}


struct Menu {
    WINDOW*               _win;
    std::vector<MenuItem> items;
    int                   selected = 0;

    Menu(WINDOW* w, std::vector<MenuItem> its)
        : _win(w), items(std::move(its))
    {
        keypad(_win, TRUE);
        // land on first non-separator
        while (selected < (int)items.size() && items[selected].separator) selected++;
    }

    void draw(bool focused = true) const {
        werase(_win);
        int rows = getmaxy(_win);
        int cols = getmaxx(_win);
        for (int i = 0; i < (int)items.size() && i < rows; ++i) {
            const auto& it = items[i];
            if (it.separator) {
                wattron(_win, COLOR_PAIR(CP_BORDER));
                mvwhline(_win, i, 0, ACS_HLINE, cols);
                wattroff(_win, COLOR_PAIR(CP_BORDER));
                continue;
            }
            bool is_sel = focused && (i == selected);
            if (is_sel) {
                wattron(_win, COLOR_PAIR(CP_SELECTED) | A_BOLD);
                mvwprintw(_win, i, 0, "%-*s", cols, (" " + it.label).c_str());
                if (!it.submenu.empty()) mvwaddch(_win, i, cols - 1, '>');
                wattroff(_win, COLOR_PAIR(CP_SELECTED) | A_BOLD);
            } else {
                wattron(_win, COLOR_PAIR(CP_NORMAL));
                mvwprintw(_win, i, 0, " %-*s", cols - 1, it.label.c_str());
                if (!it.submenu.empty()) mvwaddch(_win, i, cols - 1, '>');
                wattroff(_win, COLOR_PAIR(CP_NORMAL));
            }
        }
        wnoutrefresh(_win);
    }

    // Returns: 0=stay, 1=switch-to-right-panel, 2=quit
    // out_action is filled when a leaf menu item with an action is triggered
    int handle_key(int key, std::function<void()>& out_action) {
        int n = (int)items.size();
        out_action = nullptr;

        if (key == KEY_UP) {
            do { selected = (selected - 1 + n) % n; } while (items[selected].separator);
        } else if (key == KEY_DOWN) {
            do { selected = (selected + 1) % n; } while (items[selected].separator);
        } else if (key == KEY_RIGHT || key == '\n' || key == KEY_ENTER) {
            const auto& it = items[selected];
            if (!it.submenu.empty()) {
                // Open submenu overlay positioned to the right of this menu window
                int sy = getbegy(_win) + selected;
                int sx = getbegx(_win) + getmaxx(_win);
                auto result = run_submenu_overlay(it.submenu, sy, sx);
                if (result) out_action = result;
            } else if (key == KEY_RIGHT) {
                return 1;  // move focus to content panel
            } else {
                // Enter on a leaf item with no submenu → switch screen
                out_action = it.action;
                return it.action ? 0 : 1;  // if no action, enter = go right
            }
        } else if (key == 'q' || key == 27) {
            return 2;  // quit
        }
        return 0;
    }
};


struct BarGraph {
    struct Bar { std::string label; double value; };

    WINDOW*          _win;
    std::vector<Bar> bars;
    double           max_value = 100.0;
    bool             horizontal = false;

    BarGraph(WINDOW* w, bool horiz = false) : _win(w), horizontal(horiz) {}

    void set_bars(const std::vector<Bar>& b) {
        bars = b;
        max_value = 0;
        for (auto& bar : bars) max_value = std::max(max_value, bar.value);
        if (max_value == 0) max_value = 1;
    }

    void draw() const {
        werase(_win);
        int rows = getmaxy(_win);
        int cols = getmaxx(_win);
        if (horizontal) draw_horizontal(rows, cols);
        else            draw_vertical(rows, cols);
        wnoutrefresh(_win);
    }

private:
    void draw_horizontal(int rows, int cols) const {
        int bar_area = cols - 14;
        if (bar_area < 1) bar_area = 1;
        for (int i = 0; i < (int)bars.size() && i < rows; ++i) {
            const auto& b = bars[i];
            int filled = (int)((b.value / max_value) * bar_area);
            wattron(_win, COLOR_PAIR(CP_LABEL));
            mvwprintw(_win, i, 0, "%-10.10s", b.label.c_str());
            wattroff(_win, COLOR_PAIR(CP_LABEL));
            wattron(_win, COLOR_PAIR(CP_GRAPH) | A_BOLD);
            for (int j = 0; j < filled; ++j)
                mvwaddch(_win, i, 10 + j, ACS_BLOCK);
            wattroff(_win, COLOR_PAIR(CP_GRAPH) | A_BOLD);
            wattron(_win, COLOR_PAIR(CP_NORMAL));
            mvwprintw(_win, i, 10 + bar_area + 1, "%.1f", b.value);
            wattroff(_win, COLOR_PAIR(CP_NORMAL));
        }
    }

    void draw_vertical(int rows, int cols) const {
        if (bars.empty()) return;
        int n       = (int)bars.size();
        int bar_w   = std::max(1, (cols - n) / n);
        int graph_h = rows - 2;
        for (int i = 0; i < n; ++i) {
            const auto& b = bars[i];
            int filled = (int)((b.value / max_value) * graph_h);
            int bx = i * (bar_w + 1);
            wattron(_win, COLOR_PAIR(CP_GRAPH) | A_BOLD);
            for (int r = 0; r < filled; ++r)
                for (int bw = 0; bw < bar_w; ++bw)
                    mvwaddch(_win, graph_h - 1 - r, bx + bw, ACS_BLOCK);
            wattroff(_win, COLOR_PAIR(CP_GRAPH) | A_BOLD);
            wattron(_win, COLOR_PAIR(CP_LABEL));
            mvwprintw(_win, rows - 2, bx, "%-*.*s", bar_w, bar_w, b.label.c_str());
            mvwprintw(_win, rows - 1, bx, "%-*.0f",  bar_w, b.value);
            wattroff(_win, COLOR_PAIR(CP_LABEL));
        }
    }
};


struct Dialog {
    static int show(const std::string& title,
                    const std::string& message,
                    const std::vector<std::string>& buttons = {"OK"})
    {
        int btn_total = 0;
        for (auto& b : buttons) btn_total += (int)b.size() + 4;
        int w = std::max({(int)message.size() + 4,
                          btn_total + 2,
                          (int)title.size() + 4});
        int h = 7;
        int y = (LINES - h) / 2;
        int x = (COLS  - w) / 2;

        WINDOW* win = newwin(h, w, y, x);
        keypad(win, TRUE);
        wbkgd(win, COLOR_PAIR(CP_DIALOG));

        auto redraw = [&](int sel) {
            werase(win);
            wattron(win, COLOR_PAIR(CP_DIALOG));
            box(win, 0, 0);
            wattron(win, A_BOLD);
            mvwprintw(win, 0, (w - (int)title.size() - 2) / 2, " %s ", title.c_str());
            wattroff(win, A_BOLD);
            mvwprintw(win, 2, (w - (int)message.size()) / 2, "%s", message.c_str());
            int bx = (w - btn_total) / 2;
            for (int i = 0; i < (int)buttons.size(); ++i) {
                bool s = (i == sel);
                wattron(win, COLOR_PAIR(s ? CP_SELECTED : CP_DIALOG) | (s ? A_BOLD : 0));
                mvwprintw(win, h - 2, bx, "[ %s ]", buttons[i].c_str());
                wattroff(win, COLOR_PAIR(s ? CP_SELECTED : CP_DIALOG) | (s ? A_BOLD : 0));
                bx += (int)buttons[i].size() + 4;
            }
            wattroff(win, COLOR_PAIR(CP_DIALOG));
            wnoutrefresh(win);
            doupdate();
        };

        int sel = 0;
        redraw(sel);
        int key;
        while ((key = wgetch(win)) != ERR) {
            if (key == KEY_LEFT  && sel > 0)                     sel--;
            if (key == KEY_RIGHT && sel < (int)buttons.size()-1) sel++;
            if (key == '\n' || key == KEY_ENTER) break;
            if (key == 27) { sel = -1; break; }
            if (key == '\t') sel = (sel + 1) % (int)buttons.size();
            redraw(sel);
        }
        werase(win);
        wnoutrefresh(win);
        doupdate();
        delwin(win);
        return sel;
    }
};

struct InputBox {
    static std::string show(const std::string& title,
                            const std::string& prompt,
                            const std::string& initial = "")
    {
        int w = std::max(40, (int)prompt.size() + 6);
        int h = 5;
        int y = (LINES - h) / 2;
        int x = (COLS  - w) / 2;

        WINDOW* win = newwin(h, w, y, x);
        keypad(win, TRUE);
        wbkgd(win, COLOR_PAIR(CP_DIALOG));
        curs_set(1);

        std::string buf = initial;

        auto redraw = [&]() {
            werase(win);
            wattron(win, COLOR_PAIR(CP_DIALOG));
            box(win, 0, 0);
            wattron(win, A_BOLD);
            mvwprintw(win, 0, (w - (int)title.size() - 2) / 2, " %s ", title.c_str());
            wattroff(win, A_BOLD);
            mvwprintw(win, 2, 2, "%s", prompt.c_str());
            wattron(win, COLOR_PAIR(CP_SELECTED));
            mvwprintw(win, 3, 2, "%-*s", w - 4, buf.c_str());
            wattroff(win, COLOR_PAIR(CP_SELECTED));
            wattroff(win, COLOR_PAIR(CP_DIALOG));
            wmove(win, 3, 2 + (int)buf.size());
            wnoutrefresh(win);
            doupdate();
        };

        redraw();
        bool cancelled = false;
        int key;
        while ((key = wgetch(win)) != ERR) {
            if (key == '\n' || key == KEY_ENTER) break;
            if (key == 27) { cancelled = true; break; }
            if ((key == KEY_BACKSPACE || key == 127) && !buf.empty())
                buf.pop_back();
            else if (key >= 32 && key < 127 && (int)buf.size() < w - 5)
                buf += (char)key;
            redraw();
        }

        curs_set(0);
        werase(win);
        wnoutrefresh(win);
        doupdate();
        delwin(win);
        return cancelled ? "" : buf;
    }
};