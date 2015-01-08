#ifndef _DS18X20LIBTEST_H_
#define _DS18X20LIBTEST_H_

#define __STDC_LIMIT_MACROS

#ifdef __cplusplus
extern "C" {
#endif

void event(uint8_t,uint32_t);

#ifdef __cplusplus
}
#endif


#define UNDEFINED UINT32_MAX

#define INTERRUPTS 0,UNDEFINED
#define NO_INTERRUPTS 1,UNDEFINED
#define POWER 2
#define DIRECTION 3
#define READ_PIN 4,UNDEFINED

#define DELAY_MS 10
#define DELAY_US 11

#endif
