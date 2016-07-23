#include "tui.h"

#include <ncurses.h>

void tui_init(TUI* tui)
{
    textarea_init(&(tui->msg_area), 100, COLS, LINES-2);
    textfield_init(&(tui->input_field), 510, COLS);
}

void tui_resize(TUI* tui)
{
    tui->input_field.width = COLS;
    tui->msg_area.height = LINES - 2;
    tui->msg_area.width = COLS;
}

char* tui_input_buffer(TUI* tui)
{
    return tui->input_field.buffer;
}

void tui_add_line(TUI* tui, char* msg, int msg_len)
{
    textarea_add_line(&(tui->msg_area), msg, msg_len);
}

void tui_reset_input_buffer(TUI* tui)
{
    textfield_reset(&(tui->input_field));
}

KeypressRet tui_keypressed(TUI* tui)
{
    int ch = getch();
    return textfield_keypressed(&(tui->input_field), ch);
}

void tui_draw(TUI* tui)
{
    textarea_draw(&(tui->msg_area), 0, 0);
    textfield_draw(&(tui->input_field), 0, LINES - 2);
}
