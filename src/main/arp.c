/**
        * do a broadcast in the network to request a valid mac-adress for given IP
 */

#define DEBUG
#include "debug.h"

#include <inttypes.h>
#include <string.h>


#define MAC_LEN	6
#define IP_LEN 4
#define MAX_ARP_ENTRIES 10

static uint8_t ip_addr[MAX_ARP_ENTRIES][4];
static uint8_t mac_addr[MAX_ARP_ENTRIES][6];
static uint8_t ptr;


static void print_entry(int entry) 
{
			for (int j=0;j<MAC_LEN-1;j++)
				debugc("%02x:",mac_addr[entry][j]);
			debugc("%02x ",mac_addr[entry][MAC_LEN-1]);
			debugc(" ");
			for (int j=0;j<IP_LEN-1;j++)
				debugc("%d.",ip_addr[entry][j]);
			debugc("%d",ip_addr[entry][3]);
			debugnl();
}
static void print_arp() 
{
	debug("ARP Table");
	for(int i=0;i<MAX_ARP_ENTRIES;i++) {
			debugc("%02d ",i);
			print_entry(i);
	}
}
static uint8_t find(uint8_t* stor, uint8_t* entry, uint8_t len)
{
	for (int i=0;i<MAX_ARP_ENTRIES;i++){
        if (! strncmp((char*)stor + i*len, (char*)entry, len)) {
			debug("we found at %d",i);
            return i + 1;
		}
    }

    return 0;
}

static uint8_t find_mac(uint8_t* entry)
{
    return find((uint8_t*)mac_addr, entry,MAC_LEN );
}

static uint8_t find_ip(uint8_t* entry)
{
    return find((uint8_t*)ip_addr, entry, IP_LEN);
}

static void update_table(uint8_t* ip, uint8_t* mac, int pos)
{
	memcpy( ip_addr + pos,ip, IP_LEN);
    memcpy( mac_addr + pos,mac, MAC_LEN);
	print_arp();
}

void arp_insert(uint8_t* ip, uint8_t* mac)
{
    uint8_t f1,f2;

	// TODO: aktuell wird PTR benutzt um FCFS zu realisieren. Effizienter wäre eine zugriffsbasierte Alg. z.b. mit Sortieren nach Zugriffen, der mit den
	// wenigsten fällt raus.
	uint8_t pos=ptr;

    if ((f1 = find_ip(ip))) { //IP schon vorhanden
		error("ip already in Arp Table");
		print_entry(f1-1);
		pos=f1-1;
    } else {
        if ((f2=find_mac(mac))) { // MAC schon vorhanden
            error("duplicate MAC Arp Entry");
			print_entry(f2-1);
			pos=f2-1;
		} else { // weder IP noch MAC bereits vorhanden
        	if (++ptr >= MAX_ARP_ENTRIES)
            	ptr = 0;
		}
	}
	update_table(ip,mac,pos);
}

void arp_request(uint8_t* ip, uint8_t* mac)
{
    // send request
}


void arp_lookup(uint8_t* ip, uint8_t* mac)
{
	int pos;
	if((pos=find_ip(ip))) {
		debug("%p",mac_addr[pos-1]);
		debug("copy from table");
		memcpy(mac,mac_addr[pos-1],MAC_LEN);
		return;
	}


    /*
     * am einfachsten ist folgender vorschlag
     * 1. query in table; wenn da -> alles ok
     * 2. wenn nicht da -> starte arp_request im lan
     * 3. über einen IRQ wird die arp-antwort (wenn sie kommt) in die tabelle eingetragen
     * 4. Polle die Tabelle bis zu einem Timeout
     */
}
