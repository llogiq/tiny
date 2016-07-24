#include "server_comms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_BOLD_RED "\x1b[1;31m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#define ASSERT_(cnd, args...) \
    if (!(cnd)) { \
        fail = 1; \
        fprintf(stderr, __FILE__ ":%d ", __LINE__); \
        fprintf(stderr, ANSI_COLOR_BOLD_RED); \
        fprintf(stderr, args); \
        fprintf(stderr, ANSI_COLOR_RESET); \
    }

static int fail = 0;

void msg_parse_1();
void msg_parse_2();
void msg_parse_3();

int main()
{
    msg_parse_1();
    msg_parse_2();
    msg_parse_3();

    // char* msg3 = ":tiny_test!~tiny@213.153.193.52 JOIN #haskell\r\n"
    //              ":verne.freenode.net NOTICE * :*** Couldn\'t look up your hostname\r\n";
    // memcpy(comms.buf, msg3, strlen(msg3));
    // comms.buf_len = strlen(msg3);
    // ret = server_comms_parse_msg(&comms, &msg);
    // ASSERT_(ret == ParseOk, "msg3_1: Parse failed %d\n", ret);

    // ret = server_comms_parse_msg(&comms, &msg);
    // ASSERT_(ret == ParseOk, "msg3_2: Parse failed %d\n", ret);

    // ret = server_comms_parse_msg(&comms, &msg);
    // ASSERT_(ret == ParseTryAgain, "msg3_3: Parse failed %d\n", ret);

    if (fail)
        fprintf(stdout, ANSI_COLOR_BOLD_RED "Test failed.\n" ANSI_COLOR_RESET);
    return fail;
}

void msg_parse_1()
{
    ServerComms comms;
    server_comms_init(&comms, "test_server", "1234");

    char* msg_str = ":tiny_test!~tiny@213.153.193.52 JOIN #haskell\r\n";
    memcpy(comms.buf, msg_str, strlen(msg_str));
    comms.buf_len = strlen(msg_str);

    Msg msg;
    ParseRet ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "msg1: Parse failed: %d\n", ret);
    ASSERT_(comms.buf_len == 0, "msg1: Wrong buf_len: %d\n", comms.buf_len);
}

void msg_parse_2()
{
    ServerComms comms;
    server_comms_init(&comms, "test_server", "1234");

    char* msg_str = ":verne.freenode.net NOTICE * :*** Couldn\'t look up your hostname\r\n";
    memcpy(comms.buf, msg_str, strlen(msg_str));
    comms.buf_len = strlen(msg_str);

    Msg msg;
    ParseRet ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "msg2: Parse failed %d\n", ret);
    ASSERT_(strcmp(msg.pfx.pfx, "verne.freenode.net") == 0,
            "msg2: Wrong prefix: %s\n", msg.pfx.pfx);
    ASSERT_(strcmp(msg.cmd.str_cmd, "NOTICE") == 0,
            "msg2: Wrong command: %s\n", msg.cmd.str_cmd);
    ASSERT_(strcmp(msg.params.params, "* :*** Couldn\'t look up your hostname") == 0,
            "msg2: Wrong command: %s\n", msg.params.params);

    ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseTryAgain, "msg2_2: Parse failed %d\n", ret);
}

void msg_parse_3()
{
    ServerComms comms;
    server_comms_init(&comms, "test_server", "1234");

    char* msg1 = ":tiny_test!~tiny@213.153.193.52 JOIN #haskell\r\n";
    char* msg2 = ":verne.freenode.net NOTICE * :*** Couldn\'t look up your hostname\r\n";
    char* msg_str = malloc(sizeof(char) * (strlen(msg1) + strlen(msg2)));
    strcat(msg_str, msg1);
    strcat(msg_str, msg2);

    memcpy(comms.buf, msg_str, strlen(msg_str));
    comms.buf_len = strlen(msg_str);

    Msg msg;
    ParseRet ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "msg3_1: Parse failed %d\n", ret);
    ASSERT_((unsigned long)comms.buf_len == strlen(msg_str) - strlen(msg1),
            "msg3_1: wrong buf_len: %d\n", comms.buf_len);
    ASSERT_(strcmp(msg.pfx.pfx, "tiny_test!~tiny@213.153.193.52") == 0,
            "msg3_1: Wrong prefix: %s\n", msg.pfx.pfx);
    ASSERT_(strcmp(msg.cmd.str_cmd, "JOIN") == 0,
            "msg3_2: Wrong command: %s\n", msg.cmd.str_cmd);
    ASSERT_(strcmp(msg.params.params, "#haskell") == 0,
            "msg3_2: Wrong params: %s\n", msg.params.params);
    ASSERT_(strcmp(comms.buf, msg2) == 0,
            "msg3_2: Wrong post-parse buffer: %s\n", comms.buf);

    ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseOk, "msg3_2: Parse failed %d\n", ret);
    ASSERT_(comms.buf_len == 0,
            "msg3_2: wrong buf_len: %d\n", comms.buf_len);

    ret = server_comms_parse_msg(&comms, &msg);
    ASSERT_(ret == ParseTryAgain, "msg3_3: Parse failed %d\n", ret);
}
