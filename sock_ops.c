#define _POSIX_C_SOURCE 200809L

#include <net/if.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "sock_ops.h"

int make_socket(char *iface) {
    int conn;

    conn = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (conn == -1) {
        perror("socket failed. Check you have given superuser privileges");
        exit(0);
    }

    //char * iface = "wlp2s0";
    size_t len = strnlen(iface, IFNAMSIZ);

    if (len == IFNAMSIZ) {
        printf("Interface name too long");
        exit(0);
    }

    // bind is also valid
    if (setsockopt(conn, SOL_SOCKET, SO_BINDTODEVICE, iface, len) == -1) {
        perror("Something went wrong");
        exit(0);
    }

    return conn;
}

uint8_t * read_frame(int conn) {
    uint8_t *buffer = malloc(ETHER_MAX_LEN);

    // that 0 means no flags
    // consider a packet that is too big
    ssize_t bytes = recv(conn, buffer, ETHER_MAX_LEN, 0);
    if (bytes == -1) {
        perror("wrong");
        // for now; we could print a message
        exit(0);
    }

    printf("recived %ld bytes\n", bytes);

    return buffer;
}

void set_promiscuous(int conn, char * if_name) {
    struct ifreq ifr;
    strncpy(ifr.ifr_name, if_name, IF_NAMESIZE);

    ifr.ifr_flags = IFF_PROMISC;
    if (ioctl(conn, SIOCSIFFLAGS, &ifr) == -1) {
        perror("Ioctl failed");
        exit(0);
    }
}
