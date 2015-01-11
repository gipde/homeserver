#ifndef _DELAY_H_
#define _DELAY_H_

#include "global.h"
#include <util/delay.h>

#ifdef _TESTBUILD_
#define DELAY_HOOK_US(us) delay_hook_us(us)
#define DELAY_HOOK_MS(ms) delay_hook_ms(ms)
#else
#define DELAY_HOOK_US(us)
#define DELAY_HOOK_MS(ms)
#endif

#define delay_us(us) _delay_us(us); DELAY_HOOK_US(us)
#define delay_ms(ms) _delay_ms(ms); DELAY_HOOK_MS(ms)

#endif
