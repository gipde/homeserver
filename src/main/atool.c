/*
 * TODO
 *  Handshake zum flashen
 *      1. send Magic-Flash
 *      2. rec Begin-Flash
 *      3. send Flash-File
 *
 *  Sha verify
 *
 *  Error Handling is grausam
 *
 *  Logging ins File rein
 *
 *  Beshreibung rs232 parameter
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
        gen_head(head, "", "DEBUG ");
        break;

    case '3':
        gen_head(head, "", "INFO  ");
        break;

    default:
        ind = 0;
        gen_head(head, "", "INFO  ");
    }

    printf("%s %s \x1b[37;0m\n", head, msg + ind);
    fprintf(f, "%s %s \x1b[37;0m\n", head, msg + ind);
}

void usage(void)
{
    printf("usage: atool [-l|-f <filename>] -d <device> -b <baud>\n");
    printf("   -l  				log events from device\n");
    printf("   -f <filename>	flash file into controller\n\n");
}

void error_exit(char* msg)
{
    perror(msg);
    usage();
    exit(1);
}


int open_port(char* dev)
{
    int fd;

    printf("open Port %s... \n", dev);

    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        perror("open_port: Unable to open file");
        exit(1);
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
        error("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    tty.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                     INLCR | PARMRK | INPCK | ISTRIP | IXON);

    tty.c_oflag = 0;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= CS8;

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        error("error %d setting term attributes", errno);

    return 0;
}

FILE* logfile;

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

        if (buf == 0x0a || lineptr == 1024) {
            line[lineptr] = 0x0;
            log_msg(line, logfile);
            lineptr = 0;
        } else if (buf != 0x0d) {
            line[lineptr++] = buf;
        }

    }
}

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        fclose(logfile);
        exit(0);
    }
}


int main(int argc, char* argv[])
{

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    int log = 0, flash = 0;
    int dev = 0, baud = 0;

    char* fname;
    FILE* fp;
    int devp = 0;

    int opt;

    while ((opt = getopt(argc, argv, "lf:d:b:")) != -1) {
        switch (opt) {
        case 'l':
            log = 1;
            break;

        case 'f':
            flash = 1;
            //TODO: nullcheck
            fp = fopen(optarg, "r");

            if (fp == NULL)  {
                error("could not open file: %d", errno);
                exit(1);
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
        error_exit("specify ether log or flash");


    if (log) {
        do_log(devp);
    } else {
        int buf[1];
        printf("Sending ... \n");
        char ch;

        while ( (ch = fgetc(fp)) != EOF) {
            printf("%c", ch);
            int b = write(devp, &ch, 1);

            if (b != 1)
                error_exit("Error transmitting flash");
        }

        fclose(fp);
    }

}
