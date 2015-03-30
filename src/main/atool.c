/*
 *  Error Handling is grausam
 *  Beschreibung rs232 parameter kontrollieren, da immer noch minicom notwendig ist.
 *  Pagesize konfigurierbar machen
 *
 *  Die Programmierung erfolgt ohne eine Verbindungsschicht (vgl. UDP) es wird nur
 *  gesendet, so dass ein weiteres atool den lesenden Zugriff nicht unterbrechen
 *  braucht. Damit entfällt auch das programmieren eines Interrupts für UART im
 *  Hauptprogramm.
 */


/*
Intel Hex Format

:               Recordmark
Len             hex 2 byte
offset          16 Bit adresse (4 Byte)
rectype         2 Byte
                00 Data
                01 EOF
                02 Extended Segment
                03 Start Segment
                04 Extended Linear Segment
data            in hex bytes
checksum        2 bytes
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

#define PAGE_SIZE 128

/**
 * generate head for log statements
 */
void gen_head(char* head, char* col, char* lev)
{
    time_t t = time(NULL);
    struct tm* t2 = localtime(&t);
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    sprintf(head, "%s%20s%03.0f %s", col, " ", spec.tv_nsec / 1.0e6, lev);
    strftime(head + strlen(col), 20, "%d.%m.%Y %H:%M:%S", t2);
    head[19 + strlen(col)] = '.';
}

/**
 * log Mesage to console and to file
 */
void log_msg(char* msg, FILE* f)
{

    char head[36];
    int ind = 1;

    switch (msg[0]) {
    case '0':
        gen_head(head, "\x1b[31;1m", "ERROR ");
        break;

    case '1':
        gen_head(head, "\x1b[33;1m", "WARN  ");
        break;

    case '2':
        gen_head(head, "", "INFO  ");
        break;

    case '3':
        gen_head(head, "", "DEBUG ");
        break;

    default:
        ind = 0;
        gen_head(head, "", "INFO  ");
    }

    printf("%s %s \x1b[37;0m\n", head, msg + ind);
    fprintf(f, "%s %s \x1b[37;0m\n", head, msg + ind);
}

/**
 * print program usage
 */
void usage(void)
{
    printf("usage: atool [-l|-f <filename>] -d <device> -b <baud>\n");
    printf("   -l  				log events from device\n");
    printf("   -f <filename>	flash file into controller\n\n");
    printf("   -r               reset controller\n");
}

/**
 * exit fn
 */
void error_exit(char* msg)
{
    perror(msg);
    usage();
    exit(1);
}

/**
 * setup UART
 */
int open_port(char* dev)
{
    int fd;

    printf("open Port %s... \n", dev);

    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        error_exit("open_port: Unable to open file");
    } else
        fcntl(fd, F_SETFL, 0);

    return (fd);
}

int init_port(int fd, int baud)
{
    printf("init device with %d baud ...\n", baud);

    if (fd == 0)
        error_exit("invalid devie specified");

    struct termios tty;
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
        //error("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

	/* input modes */
    tty.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                     INLCR | PARMRK | INPCK | ISTRIP | IXON);

	/* output modes */
    tty.c_oflag = 0;

	/* local modes */
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

	/* control modes */
    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= CS8;

	/* special character */
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf("error %d setting term attributes", errno);

    return 0;
}

FILE* logfile;

/**
 * primary log method
 */
void do_log(int devp)
{

    logfile = fopen("atool.log", "a");
    fprintf(logfile, "\n---starting---\n");

    printf("Receiving ... \n");
    unsigned char buf;

    char line[1024];
    int lineptr = 0;

    for (;;) { //endless
        int c = read(devp, &buf, 1);

        // maxline = 1024 chars
        if (buf == 0x0a || lineptr == 1024) {
            line[lineptr] = 0x0;
            log_msg(line, logfile);
            lineptr = 0;
        } else if (buf != 0x0d) {
            line[lineptr++] = buf;
        }
    }
}

/**
 * close file on ctrl-c
 */
void sig_handler(int signo)
{
    if (signo == SIGINT) {
        fclose(logfile);
        exit(0);
    }
}

/**
 * calculate crc16
 */
uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    int i;
    crc ^= a;

    for (i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }

    return crc;
}

/**
 * write to uart
 */
void write_uart(int devp, unsigned char* c, int len)
{
    /*
    printf("%d: ", len);

    for (int i = 0; i < len; i++)
        printf("%x ", c[i]);

    printf("\n");
    */
    int b = write(devp, c, len);

    if (b != len)
        printf("Error transmitting %d bytes\n", len);

    usleep(1000 * 30);
}

typedef struct page {
    uint16_t no ;
    uint16_t size ;
    uint16_t crc;
    struct page* next ;
    unsigned char buf[PAGE_SIZE];
} page_t;

page_t* pageptr = NULL;
page_t* start = NULL;
uint16_t pagescount = 0;

/**
 * programming the controller
 */
int do_program(int devp)
{

    int buf[1];
    printf("Sending ... \n");

	// Transmit Character that forces soft-reset via USART_RXC intr 
	write_uart(devp,"x",1);
	sleep(1);	

    write_uart(devp, "BOOTLOADER_START", 16);

    write_uart(devp, (unsigned char*)&pagescount, 2);

    page_t* p = start;
    page_t* next;

    do {

        printf("Write page %d ... \n", p->no);
        // page nr
        write_uart(devp, (unsigned char*)&p->no, 2);

        // page size
        write_uart(devp, (unsigned char*)&p->size, 2);

        // crc
        write_uart(devp, (unsigned char*)&p->crc, 2);

        // payload
        write_uart(devp, p->buf, p->size);

        page_t* pold = p;
        next = p->next;
        p = next;
        free(pold);
    } while (next != NULL);

    free(p);
}


/*
 * convert hex char to int
 */
int c_hex(char* begin, int len)
{
    char mystring[len + 1];
    mystring[len] = 0;

    memcpy(mystring, begin, len);
    char* end;
    return strtol(mystring, &end, 16);
}

/**
 * add a page to the linked list of pages
 */
void add_page(int page, unsigned char* buf, int len)
{
    // create crc
    int crc = 0;

    for (int i = 0; i < len; i++) {
        crc = crc16_update(crc, buf[i]);
    }

    // add to linked list
    page_t* p = malloc(sizeof(page_t));
    p->no = page;
    p->size = len;
    p->crc = crc;
    memcpy(p->buf, buf, len);
    p->next = NULL;

    if (pageptr != NULL)
        pageptr->next = p;
    else
        start = p;

    pageptr = p;

    pagescount++;

}

/**
 * parse the intel hex flash-file
 */
void parse(FILE* flashfile)
{
    page_t p ;
    p.no = 0;

    char* line = NULL;
    size_t len = 0;
    size_t read = 0;

    int lastpage = 0;

    unsigned char buf[PAGE_SIZE];
    int bufptr = 0;

    while ((read = getline(&line, &len, flashfile)) != -1 ) {

        // Invalid Line
        if (line[0] != ':') {
            printf("invalid line\n");
            continue;
        }

        int r_len = c_hex(line + 1, 2);
        int offset = c_hex(line + 3, 4);
        int type = c_hex(line + 7, 2);

        // EOF Record
        if (type == 1) {
            printf("Writing %d bytes at end\n", bufptr + 1);
            add_page(++lastpage, buf, bufptr + 1);
            break;
        }

        int check = r_len + (offset >> 8) + (offset & 0xff) + type;

        for (int i = 0; i < r_len; i++) {
            bufptr = (offset + i) % PAGE_SIZE;
            buf[bufptr] = c_hex(line + 9 + i * 2, 2);
            check += buf[bufptr];

            if (bufptr + 1 == PAGE_SIZE) {
                lastpage = (offset + i) / PAGE_SIZE;
                add_page(lastpage, buf, bufptr + 1);
            }
        }

        int check_given = c_hex(line + 9 + r_len * 2, 2);
        check = (~(check & 0xff) + 1) & 0xff;

        if (check_given != check) {
            printf("Invalid Checksum %02x<>%02x in line %d", check, check_given, 1);
            exit(1);
        }

    }

    if (line)
        free(line);

}

int main(int argc, char* argv[])
{

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    int log = 0, flash = 0;
    int dev = 0, baud = 0;

    FILE* flashfile;
    int devp = 0;

    int opt;

    while ((opt = getopt(argc, argv, "lf:d:b:")) != -1) {
        switch (opt) {
        case 'l':
            log = 1;
            break;

        case 'f':
            flash = 1;
            flashfile = fopen(optarg, "r");

            if (flashfile == NULL)  {
                printf("could not open file: %d", errno);
                exit(1);
            } else {
                parse(flashfile);
            }

            break;

        case 'd':
            devp = open_port(optarg);
            dev = 1;
            break;

        case 'b':
            init_port(devp, atoi(optarg));
            baud = 1;
            break;

        default:
            error_exit("unknown Option specified");

        }
    }

    if (devp == 0)
        error_exit("no Device specified");

    if (baud == 0)
        error_exit("no Baudrate specified");

    if (log + flash != 1)
        error_exit("specify log, flash or reset");


    if (log) {
        do_log(devp);
    } else if (flash) {
        do_program(devp);
    } else {
        error_exit("Error parsing command line\n");
    }

}
