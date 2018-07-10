// socket libraries
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_arp.h>
#include <assert.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include <arpa/inet.h>
#include <linux/if_ether.h>
// #include <net/ethernet.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

#include "helpers.h"

struct arp_body {
    uint8_t src_ha[6];
    uint8_t src_pa[4];
    uint8_t dst_ha[6];
    uint8_t dst_pa[4];
};

int make_socket() {
    int conn;

    conn = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (conn == -1) {
        perror("socket failed. Check you have given superuser privileges");
        exit(0);
    }

    char * iface = "wlp2s0";
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

void parse_arp(uint8_t * request) {
    struct arphdr *header = (struct arphdr *) request;

    // this can't possibly be anyhting else
    assert(ntohs(header->ar_hrd) == ARPHRD_ETHER);
    if (ntohs(header->ar_pro) == ETH_P_IP) {
        printf("iparp\n");
        assert(header->ar_hln == 6);
        assert(header->ar_pln == 4);
    } else {
        return;
    }
    int arp_type = ntohs(header->ar_op);;
    // there's also *_R* of the same
    switch (arp_type) {
        case ARPOP_REQUEST: printf("request\n");  break;
        case ARPOP_REPLY: printf("reply\n");  break;
    }

    struct arp_body *body = (struct arp_body *)(request + sizeof(struct arphdr));

    char src_ip[INET_ADDRSTRLEN] = {0};
    char * src_mac = format_mac(body->src_ha);
    if (inet_ntop(AF_INET, body->src_pa, src_ip, INET_ADDRSTRLEN) == NULL) {
        perror("bad ip?");
    }

    printf("%s %s\n", src_mac, src_ip);
}

void parse_mac(uint8_t * frame) {
    struct ethhdr *header = (struct ethhdr *) frame;

    if (header->h_source[0] == 0)
        return;

    char * src = format_mac(header->h_source);
    char * dest = format_mac(header->h_dest);

    printf("%s %s\n", src, dest);

    uint8_t *payload = frame + sizeof(struct ethhdr);

    switch (ntohs(header->h_proto)) {
        case ETH_P_IP:
            printf("ipv4"); break;
        case ETH_P_IPV6:
            printf("ipv6"); break;
        case ETH_P_ARP:
            parse_arp(payload);
            printf("arp"); break;
        default:
            printf("id: %X", ntohs(header->h_proto));
    }
    printf(" packet\n\n");
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

int main() {

}
