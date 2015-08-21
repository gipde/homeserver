#ifndef __ARP_h
#define __ARP_h

/**
 * insert a entry into the arp-table
 */
void arp_insert(uint8_t* ip, uint8_t* mac);

/**
 * do a request on the net for the given ip
 */
void arp_request(uint8_t* ip, uint8_t* mac);

/**
 * do a lookup for the given ip
 * if the entry is not in the arp-table
 * we have to request in the net
 */
void arp_lookup(uint8_t* ip, uint8_t* mac);

#endif
