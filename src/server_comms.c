#include "server_comms.h"

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define RECV_BUFFER_SIZE 1000

void server_comms_init(ServerComms* comms, char* server, char* port)
{
    comms->sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    comms->buf = malloc(RECV_BUFFER_SIZE);
    comms->server = server;
    comms->port = port;
}

int server_comms_connect(ServerComms* comms)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res;

    if (getaddrinfo(comms->server, comms->port, &hints, &res))
    {
        // abort_msg("getaddrinfo(): %s", strerror(errno) );
        // wrefresh( stdscr );
        // return;
        return -1; // check errno
    }

    return connect(comms->sock, res->ai_addr, res->ai_addrlen);
}

void server_comms_write(ServerComms* comms, const void* buf, size_t len)
{
    send(comms->sock, buf, len, 0);
}

int server_comms_read(ServerComms* comms)
{
    // FIXME: This is not correct! We may have partial messages in the buffer.
    return recv(comms->sock, comms->buf, RECV_BUFFER_SIZE, 0);
}

void server_comms_close(ServerComms* comms)
{
    close(comms->sock);
}

int parse_prefix(char* src, int src_len, char* dst);
int parse_params(char* src, int src_len, char* dst);

// Parsing starts from the beginning of the buffer. So after a successful
// parse, we need to shift the buffer.
ParseRet server_comms_parse_msg(ServerComms* comms, Msg* msg)
{
    int buf_len = comms->buf_len;
    if (buf_len == 0) return ParseTryAgain;

    int pos = 0;

    char* buf = (char*)comms->buf;
    if (buf[pos] == ':')
    {
        // parse prefix
        int prefix_ret = parse_prefix(buf + 1, buf_len - 1, msg->pfx.pfx);
        if (prefix_ret < 0) return ParseTryAgain;
        pos += prefix_ret + 2; // skip colon and whitespace
    }
    else
    {
        // no prefix, mark it as such
        msg->pfx.pfx[0] = '\0';
    }

    // parse command
    if (pos >= buf_len) return ParseTryAgain;
    if (buf[pos] >= '0' && buf[pos] <= '9')
    {
        if (pos + 2 >= buf_len) return ParseTryAgain;
        // numeric command
        int num1 = buf[pos]     - '0';
        int num2 = buf[pos + 1] - '0';
        int num3 = buf[pos + 2] - '0';
        int num = (num1 * 100) + (num2 * 10) + num3;
        msg->cmd.type = NumCommand;
        msg->cmd.num_command = num;
        pos += 4; // skip trailing space too
    }
    else
    {
        // string command
        // FIXME: rename parse_prefix, it's reused here
        int str_cmd_ret = parse_prefix(buf + pos, buf_len - pos, msg->cmd.str_command);
        if (str_cmd_ret < 0) return ParseTryAgain;
        msg->cmd.type = StrCommand;
        pos += str_cmd_ret + 1; // skip trailing space too
    }

    // parse params
    int params_ret = parse_params(buf + pos, buf_len - pos, msg->params.params);
    if (params_ret < 0) return ParseTryAgain;

    memcpy(buf, buf + pos, buf_len - pos);
    buf_len -= pos;
    return ParseOk;
}

/**
 * Returns amount of characters consumed. Returns -1 on error.
 */
int parse_prefix(char *src, int src_len, char* dst)
{
    int idx = 0;
    while (idx < src_len)
    {
        if (src[idx] == ' ')
            break;
        idx++;
    }

    if (idx == src_len)
        return -1; // error. need to recv() more, probably

    memcpy(dst, src, idx);
    dst[idx] = '\0';
    return idx;
}

/**
 * Returns amount of characters consumed. Returns -1 on error.
 */
int parse_params(char* src, int src_len, char* dst)
{
    int idx = 0;
    while (idx + 1 < src_len)
    {
        if (src[idx] == '\r' && src[idx + 1] == '\n')
            break;
        idx++;
    }
    idx++; // position of '\n'

    if (idx >= src_len) return -1;

    memcpy(dst, src, idx - 2);
    dst[idx] = '\0';
    return idx;
}
