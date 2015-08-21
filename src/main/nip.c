/*
 * nano IP
 */

/**
 * API
 * Socket - Endpoint of Connuniation
 *
 * socket(domain) LOCAL UNIX INET INET6 RAW
 *  erzeugt ein tcp,udp socket zum lesen und schreiben
 *
 * Pkt transmit
 *  recv(soket) read from socket
 *  send(soket) send msg over socket
 *
 * Stack handle pkt boundaries
 *  read(socket) read from socket
 *  write(socket) write into socket
 *
 * icmp_send
 *
 * Anforderungen an den Treiber
 * ============================
 * komplettes lesen eines Packets
 * partielles lesen eines Packets
 * komplettes schreiben eines Packets
 * partielles schreiben eines Packets
 *
 *
 * partielles lesen aus sicht des treibers
 * =======================================
 *
 * return value
 * 0 bytes      -> Fehler
 * -1 bytes     -> konnte nicht das ganze packet lesen; noch zu lesende bytes
 *  1.. bytes   -> erfolgreich das ganze packet gelesen
 *
 * differenzierung aktives lesen/intr getriebenes lesen
 *
 */

/**
 * initialize IP Stack
 */
#include "eth-driver.h"
#include "enc28j60.h" //TODO: muss raus und in eth-driver rein
#define DEBUG
#include "debug.h"


/**
 * here we have to read packets, which are signalized via IRQ
 */

static void print_buf(uint8_t* buf, uint16_t buflen)
{
    debugn("PKT (%d): ", buflen);

    for (uint16_t i = 0; i < buflen; i++)
        debugc("%x ", buf[i]);

    debugnl();
}

#define BUFLEN 512
#define ARP     0x0
#define TCP     0x1
#define UDP     0x2
#define ICMP    0x3
#define INVALID 0xfe
#define UNKNOWN 0xff

/**
 * decode the ethernet frame
 * returns protocol Type
 */
static uint8_t decode_eth(uint8_t* buf)
{
    // 0 - 5 Destination MAC
    // 6- 11 Source MAC
    // 12-13 Type / Length (IP 0x800, ARP 0x806, RARP 0x8035)
    // -1500 Payload
    /*
     * check validity of packet through mac
     * wildcard ff ff ff ff ff ff
     */
    //check if dest equals our MAC
    int i = 6;

    enc28j60_info_t* eth_info = enc28j60_get_status();

    while (i--) {
        if (buf[i] != eth_info->mac[i]) {
            error("DST MAC not OURS %x:%x:%x:%x:%x:%x <> %x:%x:%x:%x:%x:%x",
                  buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
                  eth_info->mac[0], eth_info->mac[1], eth_info->mac[2],
                  eth_info->mac[3], eth_info->mac[4], eth_info->mac[5]);
            return INVALID;
        }
    }

    debug("pkt from %x:%x:%x:%x:%x:%x", buf[6], buf[7], buf[8], buf[9],
          buf[10], buf[11]);

    if (!(buf[12] == 0x80 && buf[13] == 0x00))  {
        error("no IP Protocol %x %x", buf[12], buf[13]);
        return INVALID;
    }

    if (buf[12] == 0x080 && buf[13] == 0x06) {
        return ARP;
    }


    return 0;
}


void intr_paket_handler()
{
    uint8_t buf[BUFLEN] = {};
    int r = read_packet(buf, BUFLEN);
    //decode ethernet protocol
    uint8_t proto = decode_eth(buf);

    switch (proto) {
    case ARP:
        // Arp resolution is done only on outgoing packets
        arp_lookup();

        break;

    case TCP:
        break;

    case UDP:
        break;

    case ICMP:
        break;

    case UNKNOWN:
        break;
    }

    print_buf(buf, r);
}

void ip_init()
{
    eth_register_listener(&intr_paket_handler);
    // alternativ könnte man einen protokoll selektierer einbauen,
    // sofern neben IP auch andere Protokolle unterstützt werden sollen.

    // TODO: init
    // Set IP Adress
    // Netmask
    // Route
    // DNS
    //IP Stack verhalten
}
