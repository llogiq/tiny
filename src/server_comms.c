#include "server_comms.h"

#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void server_comms_init(ServerComms* comms, char* server, char* port)
{
    comms->sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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

int server_comms_read(ServerComms* comms, void* buf, size_t len)
{
    return recv(comms->sock, buf, len, 0);
}

void server_comms_close(ServerComms* comms)
{
    close(comms->sock);
}
