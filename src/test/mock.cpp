#include "mock.h"

//#define DEBUG
extern "C" {
#include "../main/debug.h"
#include <util/crc16.h>
}

static uint16_t crc;

void initMock()
{

}


int cmpMock(uint16_t expected)
{
    debug("CRC actual:%d expected:%d", crc, expected);
    return expected == crc;
}


void event(event_t* event)
{
    debug("E %d,%d", event->type, event->value);

    for (int i = 0; i < 5; i++)
        crc = _crc16_update(crc, *((uint8_t*)event + i));
}


