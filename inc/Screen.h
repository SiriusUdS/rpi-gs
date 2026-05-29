#pragma once
#include "tui.h"
#include <string.h>


class Screen
{
private:
    std::string m_title;
protected:
    bool m_btn_focused;
public:

    Screen(std::string title);
    virtual void draw(Panel* p, bool focused) = 0;

    virtual int  tick_interval_ms() const { return 0; }  // 0 = static screen
    virtual bool tick()                   { return false; } // return true = needs redraw
    virtual void on_enter() {}   // called when navigating TO this screen
    virtual void on_leave() {}   // called when navigating AWAY
    virtual bool handle_key(int key);

    std::string& getTitle();
    ~Screen();

    /* ex of handle
        bool handle_key(int key) override {
        if (!btn_focused) {
            if (key == KEY_DOWN)              { btn_focused = true; return true; }
            if (key == KEY_LEFT || key == 27) return false;
            return true;
        }
        if (key == 27 || key == KEY_UP) { btn_focused = false; return true; }
        if (!buttons.handle_key(key) && key == KEY_LEFT) btn_focused = false;
        return true;
    }
    */
};
