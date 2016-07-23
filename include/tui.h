#ifndef __TINY_TUI_H
#define __TINY_TUI_H

#include "textarea.h"
#include "textfield.h"

typedef struct {
    TextArea msg_area;
    TextField input_field;
} TUI;

void tui_init(TUI*);

void tui_resize(TUI*);

char* tui_input_buffer(TUI*);

void tui_add_line(TUI* tui, char* msg, int msg_len);

void tui_reset_input_buffer(TUI*);

KeypressRet tui_keypressed(TUI*);

void tui_draw(TUI*);

#endif
