#include "global.h"
#include <avr/io.h>
#include <util/delay.h>
#include "enc28j60.h"
#include <avr/interrupt.h>

#define DEBUG
#include "debug.h"



//TODO:
//      Fehlerauswertung
//      API
//          writePacket
//          recPacket
//          setMac
//          getInfo
//          pwrSave ECON2
//          satus upon interrupt
//      LINKIE Interrupt geht nicht richtig
//      Powersave geht nicht
//      WAKEONLAN

// LOW Level SPI cmds
//----------------------------
//INLINING LOW LEVEL
void spi_send(uint8_t c)
{
    SPDR = c;

    while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_read()
{
    spi_send(0x0); // needed to provide clock, so that slave can shift out
    return SPDR;
}

void cs_low()
{
    PORT_SPI &= ~(1 << SPI_SS);
}

void cs_high()
{
    PORT_SPI |= 1 << SPI_SS;
}

void init_spi()
{
    /* Set MOSI and SCK output  and SS */
    DDR_SPI |= (1 << SPI_MOSI) | (1 << SPI_SCK)  | (1 << SPI_SS);
    /* MISO Input */
    DDR_SPI &= ~(1 << SPI_MISO);

    /* Enable SPI, Master, set clock rate fck/16 */
    SPCR = (1 << SPE) | (1 << MSTR);// | (1 << SPR0)  ;
    SPSR |= 1 << SPI2X;

    cs_high();
}


// Low Level Helper
//----------------------------

static uint8_t current_bank = 0;
void select_bank(uint8_t bank)
{
    if (bank != current_bank) {
        spi_send(CMD_BFC | ECON1);
        spi_send(1 << BSEL0 | 1 << BSEL1);
        cs_high();
        cs_low();
        spi_send(CMD_BFS | ECON1);
        spi_send(bank & (1 << BSEL0 | 1 << BSEL1));
        cs_high();
        cs_low();
        current_bank = bank;
    }
}

void spi_reg_cmd(uint8_t cmd, uint8_t reg)
{
    select_bank((reg & BANK_MASK) >> 5);
    spi_send(cmd | (reg & REG_MASK));
}

void write_op(uint8_t op, uint8_t reg, uint8_t data)
{
    cli();
    cs_low();
    spi_reg_cmd(op, reg);
    spi_send(data);
    cs_high();
    sei();
}

void buf_op(uint8_t op, uint8_t* data, uint8_t len)
{
    cli();
    cs_low();
    spi_send(op);

    while (len--) {
        if (op == CMD_RBM)
            *data++ = spi_read();
        else
            spi_send(*data++);
    }

    cs_high();
    sei();
}

// SPI INSTRUCTIONS
//------------------------

uint8_t read_control_register(uint8_t reg)
{
    cli();
    cs_low();
    spi_reg_cmd(CMD_RCR, reg);
    uint8_t data = spi_read();

    if (reg & DUMMY ) { // the last was a dummy byte?
        data = spi_read();
    }

    cs_high();
    sei();
    return data;
}

#define read_buffer_memory(data,len)        buf_op(CMD_RBM,data,len)
#define write_control_register(reg,data)    write_op(CMD_WCR,reg,data)
#define write_buffer_memory(data,len)       buf_op(CMD_WBM,data,len)
#define bitfield_set(reg,mask)              write_op(CMD_BFS,reg,mask)
#define bitfield_clear(reg,mask)            write_op(CMD_BFC,reg,mask)

void soft_reset()
{
    cli();
    cs_low();
    spi_send(CMD_SRC);
    cs_high();
    sei();

    while (!(read_control_register(ESTAT) & (1 << CLKRDY)));
}


// Higher Level
//------------------------

uint16_t read_word_register(uint8_t lo, uint8_t hi)
{
    return (read_control_register(lo) |
            (read_control_register(hi) << 8));
}

void write_word_register(uint8_t lo, uint8_t hi, uint16_t data)
{
    write_control_register(lo, data & 0xff);
    write_control_register(hi, data >> 8);
}

uint16_t read_phy_register(uint8_t reg)
{
    write_control_register(MIREGADR, reg);
    bitfield_set(MICMD, 1 << MIIRD);

    while (read_control_register(MISTAT) & (1 << BUSY));

    bitfield_clear(MICMD, 1 << MIIRD);

    return read_word_register(MIRDL, MIRDH);
}

void write_phy_register(uint8_t reg, uint16_t data)
{
    write_control_register(MIREGADR, reg);
    write_word_register(MIWRL, MIWRH, data);

    while (read_control_register(MISTAT) & (1 << BUSY));
}


// High Level
//------------------------------------------------------------

static enc28j60_info_t controller_state;
uint16_t next_packet = RX_START;

uint8_t get_eir()
{
    uint8_t retval = read_control_register(EIR);

    /*
    for (int i = 0; i < 7; i++) {
        if (retval & (1 << i)) {
            if (i == 6)
                bitfield_set(ECON2, (1 << PKTDEC));

            debug("clearing bit %d", i);
            bitfield_clear(EIR, (1 << i));
        }
    }

    debug("check_clear EIR: 0x%x -> 0x%x ", retval,
          read_control_register(EIR));
    */
    return retval;
}

void print_phy()
{
    debug("PHCON1: 0x%x", read_phy_register(PHCON1));
    debug("PHSTAT1: 0x%x", read_phy_register(PHSTAT1));
    debug("PHID1: 0x%x", read_phy_register(PHID1));
    debug("PHID2: 0x%x", read_phy_register(PHID2));
    debug("PHCON2: 0x%x", read_phy_register(PHCON2));
    debug("PHSTAT2: 0x%x", read_phy_register(PHSTAT2));
    debug("PHIE: 0x%x", read_phy_register(PHIE));
    debug("PHIR: 0x%x", read_phy_register(PHIR));
    debug("PHLCON: 0x%x\n", read_phy_register(PHLCON));
}

enc28j60_info_t* enc28j60_get_status()
{
    uint8_t estat, econ1, econ2;
    uint16_t phstat2;
    estat = read_control_register(ESTAT);
    econ1 = read_control_register(ECON1);
    econ2 = read_control_register(ECON2);
    phstat2 = read_phy_register(PHSTAT2);

    //debug("::ESTAT: 0x%x, ECON1 0x%x, EON2 0x%x", estat, econ1,econ2);
    //debug("::PHSTAT: 0x%x, EIR: 0x%x, EIE 0x%x ", phstat2,read_control_register(EIR), read_control_register(EIE));
    debug("::RX %d, TX %d", controller_state.rx_bytes,
          controller_state.tx_bytes);
    //TODO: Duplex Mode ?
    /*
    debug("BUFFER_ERROR %d ",
            ((estat&(1<<BUFFER))>>BUFFER) << BUFFER_ERROR);
    debug("LATE_COLLISION %d",
            ((estat&(1<<LATECOL))>>LATECOL) << LATE_COLLISION);
    debug("TRANSMIT_ABORT %d",
            ((estat&(1<<TXABRT))>>TXABRT) << TRANSMIT_ABORT);
    debug("POWER_SAVE %d",
            ((econ2&(1<<PWRSV))>>PWRSV) << POWER_SAVE);
    debug("TRANSMIT_BUSY %d",
            ((econ1&(1<<TXRTS))>>TXRTS) << TRANSMIT_BUSY);
    debug("RECEIVE_BUSY %d",
            ((estat&(1<<RXBUSY))>>RXBUSY) << RECEIVE_BUSY);
    debug("CLOCK_READY %d",
            ((estat&(1<<CLKRDY))>>CLKRDY) << CLOCK_READY);
    debug("LINK_STATE %d",
            ((phstat2&(1<<LSTAT))>>LSTAT) << LINK_STATE);
    */
    controller_state.state =
        (((estat & (1 << BUFFER)) >> BUFFER) << BUFFER_ERROR) |
        (((estat & (1 << LATECOL)) >> LATECOL) << LATE_COLLISION) |
        (((estat & (1 << TXABRT)) >> TXABRT) << TRANSMIT_ABORT) |
        (((econ2 & (1 << PWRSV)) >> PWRSV) << POWER_SAVE) |
        (((econ1 & (1 << TXRTS)) >> TXRTS) << TRANSMIT_BUSY) |
        (((estat & (1 << RXBUSY)) >> RXBUSY) << RECEIVE_BUSY) |
        (((estat & (1 << CLKRDY)) >> CLKRDY) << CLOCK_READY) |
        (((phstat2 & (1 << LSTAT)) >> LSTAT) << LINK_STATE);

    return &controller_state;
}


void set_mac_address(uint8_t* addr)
{
    debugn("MAC Address: ");

    for (int i = 0; i < 6; i++) {
        write_control_register((uint8_t[])MAC_REGS[i], addr[i]);
        controller_state.mac[i] = addr[i];
        debugc("%x:", addr[i]);
    }

    debugnl();
}


uint8_t write_packet(uint8_t* dst, uint16_t type, uint8_t* data,
                     uint16_t len)
{
    // Packet Start / Writeptr Start
    write_word_register(EWRPTL, EWRPTH, TX_START);

    // per packet control byte
    uint8_t pcb = 0x0; // 0x0 -> use defaults in MACON3
    write_buffer_memory(&pcb, 1);

    // MAC DEST / SRC
    write_buffer_memory(dst, 6);
    write_buffer_memory(controller_state.mac, 6);

    // Length / type
    write_buffer_memory((uint8_t*)&type, 2);

    // Data
    write_buffer_memory(data, len);

    // Packet End
    len += 1 + 6 + 6 + 2;
    write_word_register(ETXNDL, ETXNDH, TX_START + len - 1 );

    // transmit
    bitfield_set(ECON1, 1 << TXRTS);

    while (read_control_register(ECON1) & (1 << TXRTS));

    uint8_t tsv[7];
    write_word_register(ERDPTL, ERDPTH, TX_START + len);
    read_buffer_memory(tsv, 7);

    // TODO: need to test
    // Transmited Bytes and Collisions
    controller_state.tx_bytes += tsv[4] | (tsv[5] << 8);
    controller_state.tx_col_count += tsv[2] & 0b00001111;

    // Erros in Transmit (bits 20-22)
    uint8_t errors = (tsv[2] & 0b01110000) > 0;
    // Retval Transmit Done and no error
    uint8_t retval = ((tsv[2] & 0b10000000) > 0) & !errors;
    debug("tsv %d %d %d %d %d %d %d, %d", tsv[0], tsv[1], tsv[2], tsv[3],
          tsv[4], tsv[5], tsv[6], retval);

    debugnl();

    return retval;

}

int16_t read_packet(uint8_t* data)
{
    //check if packet received
    if (!(read_control_register(EIR) & (1 << PKTIF))) {
        debug("no packet received ");
        return 0;
    }

    // Read at next_packet
    write_word_register(ERDPTL, ERDPTH, next_packet);

    uint8_t npacket[2];
    read_buffer_memory(npacket, 2);
    debug("cur: 0x%x next: 0x%x", next_packet,
          npacket[0] | (npacket[1] << 8));
    next_packet = npacket[0] | (npacket[1] << 8);

    uint8_t rsv[4];
    read_buffer_memory(rsv, 4);
    debug("rsv: 0x%x, 0x%x, 0x%x, 0x%x", rsv[0], rsv[1], rsv[2], rsv[3]);

    uint8_t err = rsv[2] & 0x30;

    if (err) {
        debug("ERR RD PKT: %d", err);
        return err * -1;
    }

    uint16_t len = rsv[0] | (rsv[1] << 8);
    len -= 4; // CRC is not relevant

    if (len > MAX_FRAMELEN) {
        debug("len %d > %d maxlen", len, MAX_FRAMELEN);
        len = MAX_FRAMELEN;
    }

    debug("pkt has %d bytes", len);

    read_buffer_memory(data, len);

    // Border to maxium write to
    write_word_register(ERXRDPTL, ERXRDPTH, next_packet);

    controller_state.rx_bytes += 1;
    bitfield_set(ECON2, (1 << PKTDEC));

    return len;
}


void enc28j60_power_save(uint8_t sleep)
{
    if (sleep) {
        debug("Power down ...");
        bitfield_clear(ECON1, (1 << RXEN));

        while (read_control_register(ESTAT) & (1 << RXBUSY));

        while (read_control_register(ECON1) & (1 << TXRTS));

        bitfield_set(ECON2, (1 << VRPS) | (1 << PWRSV));
    } else {
        debug("Power up ...");
        bitfield_clear(ECON2, (1 << PWRSV));

        while (!(read_control_register(ESTAT) & (1 << CLKRDY)));

        bitfield_set(ECON1, (1 << RXEN));
    }
}


void enc28j60_init()
{
    debug("Initiating enc28j60...");

    init_spi();

    soft_reset();

    //recv buffer start/end ptr
    write_word_register(ERXSTL, ERXSTH, RX_START);
    write_word_register(ERXNDL, ERXNDH, RX_END);
    write_word_register(ERXRDPTL, ERXRDPTH, RX_START);

    //tx-buf start (end and ptr will be set on wbm)
    write_word_register(ETXSTL, ETXSTH, TX_START);

    // mac receive
    write_control_register(MACON1,
                           (1 << MARXEN) | (1 << TXPAUS) | (1 << RXPAUS));

    // enable automatic padding and CRC operations
    bitfield_set(MACON3,  (1 << PADCFG0) | (1 << TXCRCEN) | (1 << FRMLNEN));

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
    write_word_register(MAMXFLL, MAMXFLH, MAX_FRAMELEN);

    set_mac_address((uint8_t[])MAC_ADDR);

    // Receive Filters
    write_control_register(ERXFCON,
                           (1 << BCEN) | (1 << MCEN) | (1 << UCEN));

    // no loopback of transmitted frames
    write_phy_register(PHCON2, HDLDIS);

    /* configure leds: led a link status and receive activity, led b transmit activity */
    write_phy_register(PHLCON,
                       (1 << STRCH) |
                       (1 << LACFG2) |
                       (1 << LBCFG2) | (1 << LBCFG1) | (1 << LBCFG0));

    // enable interrupts
    bitfield_set(EIE, (1 << PKTIE) | (1 << INTIE) | (1 << LINKIE) |
                 (1 << TXIE) | (1
                                << TXERIE) | (1 << RXERIE));
    write_phy_register(PHIE, (1 << PGEIE) | (1 << PLNKIE));

    // enable packet reception
    bitfield_set(ECON1, 1 << B_RXEN);
    bitfield_set(ECON2, 1 << AUTOINC);

    // disable Clock Output
    write_control_register(ECOCON, 0x0);

    debug("RevID: 0x%02x", read_control_register(EREVID));

}

void handle_intr()
{
    uint8_t eir = get_eir();

    if (!eir) {
        debug("Interrupt called, but eir is empty");
        return;
    }

    // Disable IRQ Global
    bitfield_clear(EIE, INTIE);

    uint8_t packet[3000];

    for (int i = 0; i < 3000; i++) {
        packet[i] = i;
    }

    if (eir & (1 << PKTIF)) {

        uint8_t c = 0;

        do {
            uint16_t len = read_packet(packet);

            if (len > 0) { //kein Error
                debugn("Packet (%d): ", len);

                for (uint16_t i = 0; i < len; i++) {
                    debugc("%02x", packet[i]);
                }

                debugnl();
            }

            c = read_control_register(EPKTCNT);
        } while (c);


    } else if (eir & (1 << DMAIF)) {
        debug("DMA Interrupt");
        bitfield_clear(EIR, 1 << DMAIF);
    } else if (eir & (1 << LINKIF)) {
        if (read_phy_register(PHSTAT2) & (1 << LSTAT))
            debug("Link UP");
        else
            debug("Link DOWN");

        read_phy_register(PHIR);
    } else if (eir & (1 << TXIF)) {
        debug("TX Interrupt");
        bitfield_clear(EIR, 1 << TXIF);
    } else if (eir & (1 << TXERIF)) {
        debug("TX Error Interrupt");
        bitfield_clear(EIR, 1 << TXERIF);
    } else if (eir & (1 << RXERIF)) {
        debug("RX Error Interrupt");
        bitfield_clear(EIR, 1 << RXERIF);
    } else
        debug("unknown Interrupt %d", eir);

    //enc28j60_get_status();

    // Enable IRQ Global
    bitfield_set(EIE, INTIE);

}

