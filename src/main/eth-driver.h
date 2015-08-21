// include the correct driver
// das ist zwar nicht ganz sauber, da das api an sich hier gekapselt sein soll.
// aber das get_eir ...
#include <inttypes.h>

/**
 * initialize the driver
 */
uint8_t eth_init_drv();

/**
 * write a Packet to wire
 */
uint8_t write_packet(uint8_t* dst, uint16_t type, uint8_t* data,
                     uint16_t len);
/**
 * read a Packet from wire
 */
int16_t read_packet(uint8_t* data, uint16_t buflen);


/**
 * IR-Handler, called via HW-IRQ
 */
void eth_handle_intr();

/**
 * register a listener, who handles packets signalized via irq
 */
void eth_register_listener(void* listener);

