#ifndef _SHA1_H_
#define _SHA1_H_

typedef struct  {
    char h[20];
    uint64_t l;
} sha1_ctx_t;

#ifdef __cplusplus
extern "C" {
#endif
    void sha1_init(sha1_ctx_t*);
    void sha1_nextBlock(sha1_ctx_t*, char*);
    void sha1_lastBlock(sha1_ctx_t*, char*, uint16_t);
#ifdef __cplusplus
}
#endif
#endif
