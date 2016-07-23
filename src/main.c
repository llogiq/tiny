#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <signal.h>

#include <ncurses.h>

#include "tui.h"
#include "settings.h"
#include "textarea.h"
#include "textfield.h"
#include "event_loop.h"
#include "server_comms.h"

// According to rfc2812, IRC messages can't exceed 512 characters - and this
// includes \r\n, which follows every IRC message.
#define RECV_BUF_SIZE 512

static char recv_buf[ RECV_BUF_SIZE ] = {0};

////////////////////////////////////////////////////////////////////////////////

void mainloop();
void abort_msg(const char* fmt, ...);
int clear_cr_nl();

////////////////////////////////////////////////////////////////////////////////

static volatile sig_atomic_t got_sigwinch = 0;

static void sigwinch_handler(int sig)
{
    (void)sig;
    got_sigwinch = 1;
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        fprintf(stderr, "Can't register SIGWINCH action.\n");
        exit(1);
    }

    initscr();
    noecho();
    keypad( stdscr, TRUE );
    curs_set( 0 );
    raw();

    start_color();
    init_pair( COLOR_CURSOR, COLOR_WHITE, COLOR_GREEN );

    mainloop();

    endwin();

    return 0;
}

void mainloop()
{
    TUI tui;
    tui_init(&tui);

    // abort_msg("Connecting..." );

    wrefresh(stdscr);

    ServerComms comms;
    server_comms_init(&comms, "chat.freenode.org", "6665");

    wrefresh(stdscr);

    EvLoop ev_loop;
    ev_loop_init(&ev_loop);
    ev_loop_add(&ev_loop, 0);
    ev_loop_add(&ev_loop, comms.sock);

    server_comms_connect(&comms);

    while (true)
    {
        Events evs;
        int poll_ret = ev_loop_poll(&ev_loop, &evs);
        if (poll_ret == -1)
        {
            if (errno == ERESTART)
            {
                // probably SIGWINCH during select()
                if (got_sigwinch == 1)
                {
                    got_sigwinch = 0;
                    endwin();
                    refresh();

                    tui_resize(&tui);

                    continue;
                }
                else
                {
                    // TODO: report this
                    break;
                }
            }
        }

        if (events_check(&evs, 0))
        {
            // stdin is ready
            KeypressRet ret = tui_keypressed(&tui);
            if (ret == SHIP_IT)
            {
                char* msg = tui_input_buffer(&tui);
                int msg_len = strlen(msg);

                // We need \r\n suffix before sending the message.
                // FIXME: This is not how you do it though.
                msg[msg_len    ] = '\r';
                msg[msg_len + 1] = '\n'; // FIXME segfault here when buf is full
                server_comms_write(&comms, msg, msg_len + 2);
                tui_add_line(&tui, msg, msg_len);
                tui_reset_input_buffer(&tui);
            }
            else if (ret == ABORT)
            {
                break;
            }
        }

        if (events_check(&evs, comms.sock))
        {
            // socket is ready
            int recv_ret = server_comms_read(&comms, recv_buf, RECV_BUF_SIZE);
            if (recv_ret == -1)
            {
                abort_msg("recv(): %s", strerror(errno) );
            }
            else if (recv_ret == 0)
            {
                abort_msg("connection closed" );
                break;
            }
            else
            {
                abort_msg("recv() got partial msg of len %d",
                          recv_ret);

                int cursor_inc = clear_cr_nl();
                tui_add_line(&tui, recv_buf, cursor_inc);
            }
        }

        // For now draw everyting from scratch on any event
        wclear(stdscr);
        tui_draw(&tui);

        wrefresh(stdscr);
    }

    server_comms_close(&comms);
}

// This is used for two things:
//
// * We don't want to print \r\n as it confuses ncurses and/or terminals
//   (cursor moves to new line etc.)
//
// * We put the null terminator for printing.
//
// * It returns length of the string, so it can be used for incrementing the
//   cursor etc.
int clear_cr_nl()
{
    for ( int i = 0; i < RECV_BUF_SIZE - 1; ++i )
    {
        if ( recv_buf[ i ] == '\r' )
        {
            recv_buf[ i     ] = 0;
            recv_buf[ i + 1 ] = 0;
            return i;
        }
        else if ( recv_buf[ i ] == '\0' )
        {
            return i;
        }
    }

    return 0;
}

void abort_msg(const char* fmt, ... )
{
    va_list argptr;
    va_start( argptr, fmt );

    // Clear the line
    for ( int i = 0; i < COLS; i++ )
    {
        mvwaddch( stdscr, LINES - 1, i, ' ' );
    }

    wmove( stdscr, LINES - 1, 0 );
    vwprintw( stdscr, fmt, argptr );

    va_end( argptr );
}
