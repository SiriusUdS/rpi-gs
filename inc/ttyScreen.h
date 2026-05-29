#pragma once
#include "Screen.h"

#include <string>
#include <vector>
#include <deque>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <pty.h> 
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>

class HandoffScreen : public Screen {
public:
    HandoffScreen(const std::string& title,
                  const std::string& executable,
                  std::vector<std::string> argv_extra = {})
        : Screen(title), m_exe(executable), m_args(std::move(argv_extra)) {}

    void draw(Panel* p, bool /*focused*/) override {
        WINDOW* w = p->inner();
        werase(w);
        int row = getmaxy(w) / 2 - 2;
        Label(w, row,   2, "Press Enter to launch:",                  CP_LABEL,  A_BOLD).draw();
        Label(w, row+1, 4, m_exe,                                     CP_NORMAL, 0).draw();
        Label(w, row+3, 2, "TUI will be restored when it exits.",     CP_BORDER, 0).draw();
        wnoutrefresh(w);
    }

    bool handle_key(int key) override {
        if (key == '\n' || key == KEY_ENTER) { launch(); return true; }
        return !(key == KEY_LEFT || key == 27);
    }

private:
    std::string              m_exe;
    std::vector<std::string> m_args;

    void launch() {
        def_prog_mode();
        endwin();
        pid_t pid = fork();
        if (pid == 0) {
            std::vector<const char*> argv;
            argv.push_back(m_exe.c_str());
            for (auto& a : m_args) argv.push_back(a.c_str());
            argv.push_back(nullptr);
            execvp(m_exe.c_str(), const_cast<char**>(argv.data()));
            _exit(127);
        }
        if (pid > 0) { int st; waitpid(pid, &st, 0); }
        reset_prog_mode();
        refresh();
    }
};