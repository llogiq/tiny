#ifndef __TINY_SERVER_COMMS_H
#define __TINY_SERVER_COMMS_H

#include <sys/socket.h>

typedef struct {
    int sock;

    /** recv() buffer. */
    void* buf;

    /**
     * Length of usable part of the buffer. We set this instead of zeroing the
     * whole buffer.
     **/
    int buf_len;

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
int server_comms_read(ServerComms*);

typedef enum {
    ParseOk = 0,
    /** Parse error! Bytes in the buffer can't be parsed. */
    ParseError = 1,
    /** Try again after more recv(). */
    ParseTryAgain = 2,
} ParseRet;

typedef struct {
    char pfx[100];
} Prefix;

typedef enum {
    StrCommand,
    NumCommand,
} CommandType;

typedef struct {
    CommandType type;
    union {
        char str_command[10];
        int num_command;
    };
} Command;

typedef struct {
    char params[1000];
} Params;

typedef struct {
    Prefix pfx;
    Command cmd;
    Params params;
} Msg;

/**
 * Try to parse a message from the internal buffer.
 */
ParseRet server_comms_parse_msg(ServerComms*, Msg*);

void server_comms_close(ServerComms*);

#endif
