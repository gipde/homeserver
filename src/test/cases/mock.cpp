#include "mock.h"

#define DEBUG
extern "C" {
#include "../../main/debug.h"
}
typedef struct  {
    char h[20];
    uint64_t l;
} sha1_ctx_t;

#define MAXBYTES 64

extern "C" {
    void sha1_init(sha1_ctx_t*);
    void sha1_nextBlock(sha1_ctx_t*, char*);
    void sha1_lastBlock(sha1_ctx_t*, char*, uint16_t);
}

static sha1_ctx_t state;
static char block[MAXBYTES];
static uint8_t usage;



static void copy_bytes(event_t* event, int from, int to)
{
    for (int i = from; i <= to; i++) {
        char c = *((char*)event + i);
        block[usage++] = c;
    }
}

static void print_sha(char* addr)
{
    debugn("result: {");

    for (int i = 0; i < 20; i++) {
        debugc("0x%x", addr[i]);

        if (i < 19) debugc(",");
    }

    debugc("}");
    debugnl();
}

void initMock()
{
    sha1_init(&state);
    usage = 0;
}

int cmpMock(char expected[])
{
    sha1_lastBlock(&state, block, usage * 8);
    print_sha(state.h);

    for (int i = 0; i < 20; i++) {
        if (state.h[i] != expected[i])  {
            debug("false %lu<>%lu", state.h[i], expected[i]);
            return false;
        }
    }

    return true;
}

void event(event_t* event)
{
//	debug("E %d,%d",event->type,event->value);
    int n_usage = usage + 5;

    if (n_usage > MAXBYTES) {
        int tx = (MAXBYTES - usage) - 1;
        copy_bytes(event, 0, tx);
        sha1_nextBlock(&state, block);
        usage = 0;
        copy_bytes(event, tx + 1, 4);
    } else {
        copy_bytes(event, 0, 4);
    }
}


