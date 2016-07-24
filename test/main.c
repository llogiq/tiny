#include "server_comms.h"

#include <string.h>
#include <stdio.h>

#define ASSERT_(cnd, args...) \
    if (!(cnd)) { \
        fail = 1; \
        fprintf(stderr, args); \
    }

static int fail = 0;

int main()
{
    ServerComms comms;
    server_comms_init(&comms, "test_server", "1234");

    char* msg1 = ":tiny_test!~tiny@213.153.193.52 JOIN #haskell\r\n";
    memcpy(comms.buf, msg1, strlen(msg1));
    comms.buf_len = strlen(msg1);

    Msg msg;
    ParseRet ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "Parse failed: %d\n", ret);

    char* msg2 = ":verne.freenode.net NOTICE * :*** Couldn\'t look up your hostname\r\n";
    memcpy(comms.buf, msg2, strlen(msg2));
    comms.buf_len = strlen(msg2);

    ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "Parse failed %d\n", ret);

    return fail;
}
