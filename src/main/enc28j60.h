// Types
typedef struct {
    uint8_t state;
#define BUFFER_ERROR    7
#define LATE_COLLISION  6
#define TRANSMIT_ABORT  5
#define POWER_SAVE      4
#define TRANSMIT_BUSY   3
#define RECEIVE_BUSY    2
#define CLOCK_READY     1
#define LINK_STATE      0
    uint8_t mac[6];
    uint16_t tx_bytes;
    uint16_t rx_bytes;
    uint16_t tx_col_count;
} enc28j60_info_t;

// API
void set_mac_address(uint8_t* addr);
uint8_t get_eir();
enc28j60_info_t* enc28j60_get_status();
void enc28j60_power_save(uint8_t sleep);

// SPI Config
#define PORT_SPI    PORTB
#define DDR_SPI     DDRB

#define SPI_INT      PB2
#define SPI_SS       PB4
#define SPI_MOSI     PB5
#define SPI_MISO     PB6
#define SPI_SCK      PB7

// SPI Commands
#define CMD_RCR     0b00000000
#define CMD_RBM     (0b00100000|0b00011010)
#define CMD_WCR     0b01000000
#define CMD_WBM     (0b01100000|0b00011010)
#define CMD_BFS     0b10000000
#define CMD_BFC     0b10100000
#define CMD_SRC     (0b11100000|0b00011111)

#define BANK_MASK   BANK3
#define REG_MASK    0b00011111

#define BANK0       0b00000000
#define BANK1       0b00100000
#define BANK2       0b01000000
#define BANK3       0b01100000
#define DUMMY       0b10000000

// Control Registers
#define ERDPTL      (0x00|BANK0)
#define ERDPTH      (0x01|BANK0)
#define EWRPTL      (0x02|BANK0)
#define EWRPTH      (0x03|BANK0)
#define ETXSTL      (0x04|BANK0)
#define ETXSTH      (0x05|BANK0)
#define ETXNDL      (0x06|BANK0)
#define ETXNDH      (0x07|BANK0)
#define ERXSTL      (0x08|BANK0)
#define ERXSTH      (0x09|BANK0)
#define ERXNDL      (0x0a|BANK0)
#define ERXNDH      (0x0b|BANK0)
#define ERXRDPTL    (0x0c|BANK0)
#define ERXRDPTH    (0x0d|BANK0)
#define ERXWRPTL    (0x0e|BANK0)
#define ERXWRPTH    (0x0f|BANK0)
#define EIE         (0x1b|BANK0)
#define  INTIE      7
#define  PKTIE      6
#define  LINKIE     4
#define  TXIE       3
#define  TXERIE     1
#define  RXERIE     0
#define EIR         (0x1c|BANK0)
#define  PKTIF      6
#define  DMAIF      5
#define  LINKIF     4
#define  TXIF       3
#define  TXERIF     1
#define  RXERIF     0
#define ESTAT       (0x1d|BANK0)
#define  BUFFER     6
#define  LATECOL    4
#define  RXBUSY     2
#define  TXABRT     1
#define  CLKRDY     0
#define ECON2       (0x1e|BANK0)
#define  AUTOINC    7
#define  PKTDEC     6
#define  PWRSV      5
#define  VRPS       3
#define ECON1       (0x1f|BANK0)
#define  TXRTS      3
#define  B_RXEN     2
#define  BSEL1      1
#define  BSEL0      0

#define ERXFCON     (0x18|BANK1)
#define  UCEN       7
#define  MCEN       1
#define  BCEN       0
#define EPKTCNT     (0x19|BANK1)

#define MACON1      (0x00|BANK2|DUMMY)
#define  TXPAUS     3
#define  RXPAUS     2
#define  PASALL     1
#define  MARXEN     0
#define MACON3      (0x02|BANK2|DUMMY)
#define  PADCFG2    7
#define  PADCFG0    5
#define  TXCRCEN    4
#define  FRMLNEN    1
#define  FULDPX     0
#define MACON4      (0x03|BANK2|DUMMY)
#define MABBIPG     (0x04|BANK2|DUMMY)
#define MAIPGL      (0x06|BANK2|DUMMY)
#define MAIPGH      (0x07|BANK2|DUMMY)
#define MACLCON1    (0x08|BANK2|DUMMY)
#define MACLCON2    (0x09|BANK2|DUMMY)
#define MAMXFLL     (0x0a|BANK2|DUMMY)
#define MAMXFLH     (0x0b|BANK2|DUMMY)
#define MICMD       (0x12|BANK2|DUMMY)
#define  MIIRD      0
#define MIREGADR    (0x14|BANK2|DUMMY)
#define MIWRL       (0x16|BANK2|DUMMY)
#define MIWRH       (0x17|BANK2|DUMMY)
#define MIRDL       (0x18|BANK2|DUMMY)
#define MIRDH       (0x19|BANK2|DUMMY)

#define MAADR5      (0x00|BANK3|DUMMY)
#define MAADR6      (0x01|BANK3|DUMMY)
#define MAADR3      (0x02|BANK3|DUMMY)
#define MAADR4      (0x03|BANK3|DUMMY)
#define MAADR1      (0x04|BANK3|DUMMY)
#define MAADR2      (0x05|BANK3|DUMMY)
#define EREVID      (0x12|BANK3)
#define MISTAT      (0x0a|BANK3|DUMMY)
#define  BUSY       0
#define ECOCON      (0x15|BANK3|DUMMY)
#define PHCON1      0x00
#define  PDPXMD     8
#define PHSTAT1     0x01
#define PHID1       0x02
#define PHID2       0x03
#define PHCON2      0x10
#define  HDLDIS     8
#define PHSTAT2     0x11
#define  LSTAT      10
#define PHIE        0x12
#define  PLNKIE     4
#define  PGEIE      1
#define PHIR        0x13
#define PHLCON      0x14
#define  STRCH      1
#define  LACFG3     11
#define  LACFG2     10
#define  LACFG1     9
#define  LACFG0     8
#define  LBCFG3     7
#define  LBCFG2     6
#define  LBCFG1     5
#define  LBCFG0     4

#define MAC_REGS    { MAADR1, MAADR2, MAADR3, MAADR4, MAADR5, MAADR6 }
#define MAC_ADDR    { 0xc2, 0x1c, 0x55, 0x6f, 0xbc, 0xab }

#define RX_START    0x0000
#define RX_END      0x0FFF
#define TX_START    0x1000
#define TX_END      0x1FFF

#define MAX_FRAMELEN    1500
