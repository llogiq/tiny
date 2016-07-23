#ifndef __TINY_EVENT_LOOP_H
#define __TINY_EVENT_LOOP_H

#include <sys/select.h>

#define MAX_EVLOOP_SOCKS 100

typedef struct {
    int socks[MAX_EVLOOP_SOCKS];

    /** Number of sockets in 'socks'. */
    int n_socks;

    /** Read file descriptors. */
    fd_set rfds;

    /** Biggest file descriptor in 'rfds'. */
    int fdmax;
} EvLoop;

typedef struct { fd_set fd_set; } Events;

void ev_loop_init(EvLoop* ev_loop);

void ev_loop_add(EvLoop* ev_loop, int fd);

/**
 * Return values:
 *
 *   -1: select() failed. Check 'errno'.
 *    0: See 'evs' for available sockets.
 */
int ev_loop_poll(EvLoop* ev_loop, Events* evs);

int events_check(Events *evs, int fd);

#endif
