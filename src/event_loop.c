#include "event_loop.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

void ev_loop_init(EvLoop* ev_loop)
{
    memset(ev_loop, 0, sizeof(EvLoop));
}

void ev_loop_add(EvLoop* ev_loop, int fd)
{
    assert(ev_loop->n_socks < MAX_EVLOOP_SOCKS - 1);

    ev_loop->socks[ev_loop->n_socks] = fd;
    ev_loop->n_socks++;
    FD_SET(fd, &(ev_loop->rfds));
    ev_loop->fdmax = (fd > ev_loop->fdmax) ? fd : ev_loop->fdmax;
}

int ev_loop_poll(EvLoop* ev_loop, Events* evs)
{
    evs->fd_set = ev_loop->rfds;

    if (select(ev_loop->fdmax + 1,
               &(evs->fd_set), // readfds
               NULL,           // writefds
               NULL,           // exceptfds
               NULL)           // timeout
            == -1) {
        return -1;
    } else {
        return 0;
    }
}

int events_check(Events *evs, int fd)
{
    return FD_ISSET(fd, &(evs->fd_set));
}
