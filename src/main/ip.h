#define BUFSIZE 1024

typedef uint8_t ip_addr[4];
typedef uint8_t ip_buf[BUFSIZE];

typedef struct {
		ip_addr	ripaddr;
		uint16_t lport;
		uint16_t rport;
		uint8_t rcv_nxt[4]; // next seq number
		uint8_t snd_nxt[4]; // seq number last sent by us
		uint16_t len;
		uint16_t mss;
		uint16_t initialmss;
		uint8_t sa; // retransmission 
		uint8_t sv; // dito
		uint8_t rto; // dito
		uint8_t tcpstateflags;
		uint8_t timer;
		uint8_t nrtx; // number of retransmissions for the last segment
} tcp_connection;

typedef struct {
		ip_addr ripaddr;
		uint16_t lport;
		uint16_t rport;
		uint8_t ttl;
} udp_connection;

typedef struct {
} ip_stats;

//Setup / internal fucntions
void init(ip_addr);
ip_addr ip_gethostaddr();
void ip_setaddr(ip_addr);
ip_addr get_router_addr();
ip_addr get_netmask();

void ip_arp_out();  //arp
void ip_arp_in(); 

void ip_periodic(int i);	// wird durch time getriggered über alle verbindungen
void ip_poll(int i); // poll from a connection explicitly

void udp_periodic(int i);


// app api
void listen(uint16_t port);
void unlisten(uint16_t port);

void connect(ip_addr*,uint16_t port);
void send(onst void *data, int len);
void close();
void stop();
void abort();

void process(); // all processing on incoming packets


// Network interface -> ggf müssen es mehrere sein, z.b. lan port, und home-rf
// network forwarding
// arp code 
// socket abstraction
