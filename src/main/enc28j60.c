#include "global.h"
#include <avr/io.h>
#include <util/delay.h>
#include "enc28j60.h"
#include <avr/interrupt.h>

//#define DEBUG
#include "debug.h"

static uint8_t MAC[] = MAC_ADDR;


//TODO: 
//		Fehlerauswertung
//		Object Size Optimization
//		API
//			writePacket
//			recPacket
//			setMac
//			getMac
//			getCounter
//			en/disableRec
//			en/disableTx
//			pwrSave ECON2
//			phyInfo PHSTAT1 / PHSTAT2
//			satus upon interrupt
//

// LOW Level SPI cmds
//----------------------------
inline void spi_send(uint8_t c)
{
    SPDR = c;
    while (!(SPSR & (1 << SPIF)));
}

inline uint8_t spi_read()
{
	spi_send(0x0); // needed to provide clock, so that slave can shift out
	return SPDR;
}

inline void cs_low()
{
    PORT_SPI &= ~(1 << SPI_SS);
}

inline void cs_high()
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
    SPSR |= 1 <<SPI2X;

    cs_high();
	debug("SPCR %d",SPSR);
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

void spi_reg_cmd(uint8_t cmd,uint8_t reg)
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
	while(len--) {
		if (op==CMD_RBM)
			*data++=spi_read();
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

inline void read_buffer_memory(uint8_t* data, uint8_t len)
{
	buf_op(CMD_RBM,data,len);
}

inline void write_control_register(uint8_t reg, uint8_t data)
{
	write_op(CMD_WCR,reg,data);
}

inline void write_buffer_memory(uint8_t* data, uint8_t len)
{
	buf_op(CMD_WBM,data,len);
}

inline void bitfield_set(uint8_t reg, uint8_t mask)
{
	write_op(CMD_BFS,reg,mask);
}

inline void bitfield_clear(uint8_t reg, uint8_t mask)
{
	write_op(CMD_BFC,reg,mask);
}

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

    return (read_control_register(MIRDL) |
            (read_control_register(MIRDH) << 8));
}

void write_phy_register(uint8_t reg, uint16_t data)
{
    write_control_register(MIREGADR, reg);
    write_word_register(MIWRL, MIWRH, data);

    while (read_control_register(MISTAT) & (1 << BUSY));
}


// High Level
//------------------------------------------------------------

void set_mac_address(uint8_t* addr)
{
	debugn("MAC Address:");
    for (int i = 0; i < 6; i++) {
        write_control_register((uint8_t[])MAC_REGS[i], addr[i]);
		MAC[i]=addr[i];
		debugc("%x:",addr[i]);
    }
	debugnl();
}

void get_mac_address(uint8_t* addr)
{
	for (uint8_t i=0;i<6;i++)
			addr[i]=read_control_register((uint8_t[])MAC_REGS[i]);
}

uint16_t get_tx_count()
{
		return 0;
}

uint16_t get_rx_count()
{
		return 0;
}

uint8_t write_packet(uint8_t* dst, uint16_t type, uint8_t* data, uint16_t len)
{
    // Packet Start / Writeptr Start
    write_word_register(EWRPTL, EWRPTH, TX_START);

    // per packet control byte
    uint8_t pcb = 0x0; // 0x0 -> use defaults in MACON3
    write_buffer_memory(&pcb, 1); 

    // MAC DEST / SRC
	write_buffer_memory(dst,6);
    write_buffer_memory(MAC, 6);

    // Length / type
	write_buffer_memory((uint8_t*)&type,2);

    // Data
	write_buffer_memory(data,len);

    // Packet End
	len += 1+6+6+2;
    write_word_register(ETXNDL, ETXNDH, TX_START+len-1 );

    // transmit
    bitfield_set(ECON1, 1 << TXRTS);

	while(read_control_register(ECON1) &(1<<TXRTS));

    debugn("TSV: ");
    write_word_register(ERDPTL, ERDPTH, TX_START+len);
    uint8_t tsv[7];
    read_buffer_memory(tsv, 7);

    for (uint8_t i = 0; i < 7; i++) {
        debugc("%0x ", tsv[i]);
    }
	// Byte Count globally
	// Errors detect

    debugnl();

    //controller write 7 byte status vecotr after packet
	
	return 0;

}

void write_test_packet() 
{
    static uint8_t pkt1[84] = {
0x45, 0x00, /* ......E. */
0x00, 0x54, 0x55, 0x68, 0x40, 0x00, 0x40, 0x01, /* .TUh@.@. */
0xe7, 0x3e, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x00, /* .>...... */
0x00, 0x01, 0x08, 0x00, 0x82, 0xaf, 0x05, 0xb5, /* ........ */
0x00, 0x01, 0x71, 0xf9, 0x18, 0x55, 0x00, 0x00, /* ..q..U.. */
0x00, 0x00, 0x18, 0x79, 0x0e, 0x00, 0x00, 0x00, /* ...y.... */
0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, /* ........ */
0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, /* ........ */
0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, /* .. !"#$% */
0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, /* &'()*+,- */
0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, /* ./012345 */
0x36, 0x37                                      /* 67 */
};
	uint8_t dst[] = {0x00,0x90,0xf5,0xd7,0x30,0x6a};

	for(int i=0;i<10;i++)
		write_packet(dst,0x08,pkt1,84);

}

void read_buffer()
{
    uint8_t buf[100] = {};

		uint16_t ptr=RX_START;
        write_word_register(ERDPTL, ERDPTH, ptr);

		while(ptr < TX_END) {
	        read_buffer_memory(buf, 100);
			ptr+=100;
    	    debugn("TX BUF %d : ", ptr);

        	for (int j = 0; j < 100; j++) {
            	debugc("%0x ", buf[j]);
	        }

    	    debugnl();
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

    set_mac_address(MAC);

    // Receive Filters
    write_control_register(ERXFCON,
                           (1 << BCEN) | (1 << MCEN) | (1 << UCEN));

    // no loopback of transmitted frames
    write_phy_register(PHCON2, HDLDIS);
    /* configure leds: led a link status and receive activity, led b transmit activity */
    write_phy_register(PHLCON,
                       (1 << STRCH) | (1 << LACFG3) | (1 << LACFG2) | (1 << LBCFG0));

    // enable interrupts
    bitfield_set(EIE, (1 << PKTIE) | (1 << INTIE) | (1 << LINKIE) |
                 (1 << TXIE) | (1
                                << TXERIF) | (1 << RXERIF));
    write_phy_register(PHIE, (1 << PGEIE) | (1 << PLNKIE));

    // enable packet reception
    bitfield_set(ECON1, 1 << B_RXEN);
    bitfield_set(ECON2, 1 << AUTOINC);

    debug("RevID: 0x%02x", read_control_register(EREVID));

    write_test_packet();
}


