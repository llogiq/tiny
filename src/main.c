#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <signal.h>

#include "tui.h"
#include "settings.h"
#include "textarea.h"
#include "textfield.h"
#include "event_loop.h"
#include "server_comms.h"

////////////////////////////////////////////////////////////////////////////////

void mainloop();
void abort_msg(const char* fmt, ...);

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

    mainloop();

    return 0;
}

void mainloop()
{
    TUI tui;
    tui_init(&tui);

    // abort_msg("Connecting..." );

    ServerComms comms;
    server_comms_init(&comms, "chat.freenode.org", "6665");

    EvLoop ev_loop;
    ev_loop_init(&ev_loop);
    ev_loop_add(&ev_loop, 0);
    ev_loop_add(&ev_loop, comms.sock);

    server_comms_connect(&comms);

    for (;;)
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
                    tui_reset(&tui);
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
            int recv_ret = server_comms_read(&comms);
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

                // TODO:
                // tui_add_line(&tui, recv_buf, cursor_inc);
            }
        }

        tui_draw(&tui);
    }

    server_comms_close(&comms);
    tui_close(&tui);
}

void abort_msg(const char* fmt, ... )
{
    va_list argptr;
    va_start( argptr, fmt );

    /*
    // Clear the line
    for ( int i = 0; i < COLS; i++ )
    {
        mvwaddch( stdscr, LINES - 1, i, ' ' );
    }

    wmove( stdscr, LINES - 1, 0 );
    vwprintw( stdscr, fmt, argptr );
    */

    va_end( argptr );
}
