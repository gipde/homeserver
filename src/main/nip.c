/*
 * nano IP
 */

/**
 * API
 * Socket - Endpoint of Connuniation
 *
 * socket(domain) LOCAL UNIX INET INET6 RAW
 * 	erzeugt ein tcp,udp socket zum lesen und schreiben
 *
 * Pkt transmit
 * 	recv(soket) read from socket
 * 	send(soket) send msg over socket
 *
 * Stack handle pkt boundaries
 * 	read(socket) read from socket
 * 	write(socket) write into socket
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
 * 0 bytes		-> Fehler
 * -1 bytes		-> konnte nicht das ganze packet lesen; noch zu lesende bytes
 *  1.. bytes   -> erfolgreich das ganze packet gelesen
 *
 * differenzierung aktives lesen/intr getriebenes lesen
 *
 */

/**
 * initialize IP Stack
 */
#include "eth-driver.h"
#define DEBUG
#include "debug.h"

/**
 * here we have to read packets, which are signalized via IRQ
 */
void intr_paket_handler() {
		debug("we ar in the int paket hdl");
}

void ip_init() {
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
