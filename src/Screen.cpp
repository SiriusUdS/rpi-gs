#include "Screen.h"

Screen::Screen(std::string title)
{
    this->m_title = title;
}

bool Screen::handle_key(int key)
{
    return !(key == KEY_LEFT || key == 27);
}

std::string& Screen::getTitle()
{
    return this->m_title;
}

Screen::~Screen()
{
}