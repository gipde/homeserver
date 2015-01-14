#define M_PINA  0x19
#define M_DDRA  0x1A
#define M_PORTA 0x1B

#ifdef _TESTBUILD_
#define DELEGATE1(fn)       fn()
#define DELEGATE2(fn,arg1)  fn(arg1)
typedef struct {
    char* data;
    int size;
} replay_T;

void power(uint8_t);
void direction(uint8_t);
uint8_t read();
void set_replay_data(replay_T*);
void interrupts();
void no_interrupts();
void delay_hook_us(uint16_t);
void delay_hook_ms(uint16_t);
#define OW_READ(ow)     read()
#else
#define DELEGATE1(fn)
#define DELEGATE2(fn,arg1)
#define OW_READ(ow) (IO(ow->pin) & (1 << ow->port_pin)) >> ow->port_pin
#endif

#define IO(port)            (*(volatile uint8_t *)((port) + 0x20))
#define OW_HIGH(ow)         IO(ow->port) |= 1 << ow->port_pin; \
                                            DELEGATE2(power,HIGH)
#define OW_LOW(ow)          IO(ow->port) &= ~(1 << ow->port_pin); \
                                            DELEGATE2(power,LOW)
#define OW_INPUT(ow)        IO(ow->port) &= ~( 1 << ow->port_pin); \
                                            DELEGATE2(direction,INPUT)
#define OW_OUTPUT(ow)       IO(ow->port) |= 1 << ow->port_pin; \
                                            DELEGATE2(direction,OUTPUT)

#define INTERRUPTS          sei(); DELEGATE1(interrupts);

#define NO_INTERRUPTS       cli(); DELEGATE1(no_interrupts);


