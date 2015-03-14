#include "global.h"
#include <avr/io.h>
#include <util/delay.h>
#include "enc28j60.h"
#include <avr/interrupt.h>

#define DEBUG
#include "debug.h"


//TODO: soft spi try

void spi_send(uint8_t c)
{
    SPDR = c;

    while (!(SPSR & (1 << SPIF)));
}

inline void cs_low()
{
    PORT_SPI &= ~(1 << DD_CS);
}

inline void cs_high()
{
    PORT_SPI |= 1 << DD_CS;
}

static uint8_t current_bank;
void select_bank(uint8_t bank)
{
    if (bank != current_bank) {
        spi_send(CMD_BFC | (ECON1 & REG_MASK));
        spi_send(0);

        cs_high();
        cs_low();

        spi_send(CMD_BFS | (ECON1 & REG_MASK));
        spi_send(bank & (1 << BSEL0 | 1 << BSEL1));

        cs_high();
        current_bank = bank;
        cs_low();

    }
}

void spi_cmd(uint8_t cmd, uint8_t reg, uint8_t data)
{
    select_bank((reg & BANK_MASK) >> 5);
    spi_send(cmd | (reg & REG_MASK));
    spi_send(data);
}

uint8_t read_control_register(uint8_t reg)
{
    cs_low();
    spi_cmd(CMD_RCR, reg, 0x0);

    uint8_t data = SPDR;

    if (reg & 0b10000000) { // the last was a dummy byte?
        spi_send(0x0);
        data = SPDR;
    }

    cs_high();
    return data;
}

void write_control_register(uint8_t reg, uint8_t data)
{
    cs_low();
    spi_cmd(CMD_WCR, reg, data);
    cs_high();
}

void bitfield_set(uint8_t reg, uint8_t mask)
{
    cs_low();
    spi_cmd(CMD_BFS, reg, mask);
    cs_high();
}

void bitfield_clear(uint8_t reg, uint8_t mask)
{
    cs_low();
    spi_cmd(CMD_BFC, reg, mask);
    cs_high();
}

uint8_t read_buffer_memory()
{
    cs_low();
    spi_send(CMD_RBM);
    spi_send(0);

    uint8_t data = SPDR;

    cs_high();
    return data;
}

void write_buffer_memory(uint8_t data)
{
    cs_low();
    spi_send(CMD_WBM);
    spi_send(data);
    cs_high();
}

void soft_reset()
{
    cs_low();
    spi_send(CMD_SRC);
    cs_high();

    while (!(read_control_register(ESTAT) & (1 << CLKRDY)));
}

void set_mac_address(uint8_t* addr)
{
    uint8_t REGS[] = MAC_REGS;

    for (int i = 0; i < 6; i++) {
        write_control_register(REGS[i], addr[i]);
    }
}

void print_mac()
{
    debug("MAC Address: %x:%x:%x:%x:%x:%x",
          read_control_register(MAADR1),
          read_control_register(MAADR2),
          read_control_register(MAADR3),
          read_control_register(MAADR4),
          read_control_register(MAADR5),
          read_control_register(MAADR6));
}

void set_buffer_pointer(uint8_t lo, uint8_t hi, uint16_t address)
{
    write_control_register(lo, address & 0xff);
    write_control_register(hi, address >> 8);
}

uint16_t get_buffer_pointer(uint8_t lo, uint8_t hi)
{
    return (read_control_register(lo) |
            (read_control_register(hi) << 8));
}

uint16_t read_phy_register(uint8_t reg)
{
    write_control_register(MIREGADR, reg);
    bitfield_set(MICMD, 1 << MIIRD);

    while (read_control_register(MISTAT) & (1 << BUSY));

    bitfield_clear(MICMD, 1 << MIIRD);

    return (read_control_register(MIRDL) |
            (read_control_register(MIRDH) << 8));
}

void write_phy_register(uint8_t reg, uint16_t data)
{
    write_control_register(MIREGADR, reg);

    write_control_register(MIWRL, data & 0xff);
    write_control_register(MIWRH, data >> 8);

    while (read_control_register(MISTAT) & (1 << BUSY));
}


void init_spi()
{
    // CS out
    DDR_SPI |= 1 << DD_CS;

    /* Set MOSI and SCK output */
    DDR_SPI |= 1 << DD_MOSI | 1 << DD_SCK;
    DDR_SPI &= ~(1 << DD_MISO);

    /* Enable SPI, Master, set clock rate fck/16 */
    SPCR = 1 << SPE | 1 << MSTR | 1 << SPR0;
}

/*
Ethernet II, Src: IntelCor_b4:13:16 (8c:a9:82:b4:13:16), Dst: Broadcast (ff:ff:ff:ff:ff:ff)
            Destination: Broadcast (ff:ff:ff:ff:ff:ff)
                Address: Broadcast (ff:ff:ff:ff:ff:ff)
                .... ..1. .... .... .... .... = LG bit: Locally administered address (this is NOT the factory default)
                .... ...1 .... .... .... .... = IG bit: Group address (multicast/broadcast)
            Source: IntelCor_b4:13:16 (8c:a9:82:b4:13:16)
                Address: IntelCor_b4:13:16 (8c:a9:82:b4:13:16)
                .... ..0. .... .... .... .... = LG bit: Globally unique address (factory default)
                .... ...0 .... .... .... .... = IG bit: Individual address (unicast)
            Type: ARP (0x0806)
        Address Resolution Protocol (request)
            Hardware type: Ethernet (1)
            Protocol type: IP (0x0800)
            Hardware size: 6
                Protocol size: 4
                    Opcode: request (1)
            Sender MAC address: IntelCor_b4:13:16 (8c:a9:82:b4:13:16)
            Sender IP address: 10.0.1.57 (10.0.1.57)
            Target MAC address: 00:00:00_00:00:00 (00:00:00:00:00:00)
            Target IP address: 10.0.1.1 (10.0.1.1)

    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8c, 0xa9, 0x82, 0xb4, 0x13, 0x16, 0x08, 0x06, 0x00, 0x01,
    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x8c, 0xa9, 0x82, 0xb4, 0x13, 0x16, 0x0a, 0x00, 0x01, 0x39,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x01

*/
typedef struct {
    uint8_t dest[6];
    uint8_t src[6];
    uint8_t type[2];
    uint8_t* payload; // 46-1500 Bytes
    uint8_t fcs[4];
} eth_frm;

void transmit_dummy()   //ARP WHO HAS?
{
    uint8_t arp[] = {0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x8c, 0xa9, 0x82, 0xb4, 0x13, 0x16, 0x0a, 0x00, 0x01, 0x39,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x01
                    };
    eth_frm f = {
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // dest
        MAC_ADDR,                        // src
        {0x08, 0x06}, // type ARP
        arp,
        {0, 0, 0, 0}
    };

    for (int i = 0; i < 18 + 28; i++) {
        write_buffer_memory(*((uint8_t*)&f + i));
    }

    debug("Transmitted Dummy... ");
}



void enc28j60_init()
{
    debug("Initiating enc28j60...");

    init_spi();

    soft_reset();

    transmit_dummy();

    //recv buffer start
    write_control_register(ERXSTL, RX_START & 0xff);
    write_control_register(ERXSTH, RX_START >> 8);
    //recv-buf end
    write_control_register(ERXNDL, RX_END & 0xff);
    write_control_register(ERXNDH, RX_END >> 8);
    //recv-ptr addr
    write_control_register(ERXRDPTL, RX_START & 0xff);
    write_control_register(ERXRDPTH, RX_START >> 8);

    //tx-buf start
    write_control_register(ETXSTL, TX_START & 0xff);
    write_control_register(ETXSTH, TX_START >> 8);
    //tx-buf end
    write_control_register(ETXNDL, TX_END & 0xff);
    write_control_register(ETXNDH, TX_END >> 8);
    //tx-ptr addr
    write_control_register(ERXWRPTL, TX_START & 0xff);
    write_control_register(ERXWRPTH, TX_START >> 8);

    // mac receive
    write_control_register(MACON1, MARXEN | TXPAUS | RXPAUS);
    // out of reset
    write_control_register(MACON2, 0x00);

    // enable automatic padding and CRC operations
    bitfield_set(MACON3, 1 << PADCFG0 | 1 << TXCRCEN | 1 << FRMLNEN);

    if (read_phy_register(PHCON1) & (1 << PDPXMD)) {

        bitfield_set(MACON3, 1 << FULDPX);

        // set inter-frame gap (back-to-back)
        write_control_register(MABBIPG, 0x12);
    } else {
        bitfield_clear(MACON3, 1 << FULDPX);

        write_control_register(MABBIPG, 0x15);
    }

    // set inter-frame gap (non-back-to-back)
    write_control_register(MAIPGL, 0x12);
    write_control_register(MAIPGH, 0x0c);

    // Set the maximum packet size which the controller will accept
    write_control_register(MAMXFLL, MAX_FRAMELEN & 0xff);
    write_control_register(MAMXFLH, MAX_FRAMELEN >> 8);

    set_mac_address((uint8_t[])MAC_ADDR);

    // Receive Filters
    write_control_register(ERXFCON, 1 << BCEN | 1 << MCEN | 1 << UCEN);

    // no loopback of transmitted frames
    write_phy_register(PHCON2, HDLDIS);
    /* configure leds: led a link status and receive activity, led b transmit activity */
    write_phy_register(PHLCON,
                       1 << STRCH | 1 << LACFG3 | 1 << LACFG2 | 1 << LBCFG0);

    // enable interrupts
    bitfield_set(EIE, 1 << PKTIE | 1 << INTIE | 1 << LINKIE | 1 << TXIE | 1
                 << TXERIF | 1 << RXERIF);
    write_phy_register(PHIE, 1 << PGEIE | 1 << PLNKIE);

    // enable packet reception
    bitfield_set(ECON1, 1 << B_RXEN);
    bitfield_set(ECON2, 1 << AUTOINC);

    debug("RevID: 0x%02x", read_control_register(EREVID));

}


