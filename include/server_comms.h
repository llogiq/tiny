#ifndef __TINY_SERVER_COMMS_H
#define __TINY_SERVER_COMMS_H

#include <sys/socket.h>

typedef struct {
    int sock;

    char* server;
    char* port;
} ServerComms;

void server_comms_init(ServerComms*, char* server, char* port);

/**
 * Does not block. Result:
 *
 *    0: OK.
 *   -1: Check errno.
 **/
int server_comms_connect(ServerComms*);

/** Non-blocking write. */
void server_comms_write(ServerComms*, const void* buf, size_t len);

/**
 * Non-blocking read.
 *
 *   -1 -> Check errno (not available)
 **/
int server_comms_read(ServerComms*, void* buf, size_t len);

void server_comms_close(ServerComms*);

#endif
