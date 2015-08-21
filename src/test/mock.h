#ifndef _MOCK_H_
#define _MOCK_H_

#define __STDC_LIMIT_MACROS

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#ifdef __cplusplus
}
#endif


typedef struct {
    uint8_t type;
    uint32_t value;
} event_t;


#ifdef __cplusplus
extern "C" {
#endif
void initMock();
void event(event_t*);
int cmpMock(uint16_t);
#ifdef __cplusplus
}
#endif

#define EVENT(...) event_t e= {__VA_ARGS__}; event(&e)

#endif
