#define _POSIX_C_SOURCE 200809L

#include "parsers.h"
#include "sock_ops.h"

#include <net/if.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/if_packet.h>

#include <net/ethernet.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define ARP_FRAME_LEN (sizeof(struct ethhdr) + sizeof(struct arp_pac))

extern uint8_t *hwaddr;
extern int if_index;

int make_socket(char * iface) {
    int conn;

    conn = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (conn == -1) {
        perror("socket failed. Check you have given superuser privileges");
        exit(0);
    }

    // char * iface = "wlp2s0";
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

void broadcast_frame(int conn, struct arp_pac * data) {
    uint8_t * payload = malloc(ARP_FRAME_LEN);
    struct ethhdr * header = (struct ethhdr *)payload;
    struct sockaddr_ll addr = {
        .sll_ifindex = if_index,
        .sll_halen = ETH_ALEN
    };

    // set boradcast info
    memset(addr.sll_addr, 0xFF, ETH_ALEN);
    memset(header->h_dest, 0xFF, ETH_ALEN);
    memcpy(header->h_source, hwaddr, ETH_ALEN);
    header->h_proto = htons(ETH_P_ARP);

    // copy payload
    memcpy(payload + sizeof(struct ethhdr), data, sizeof(struct arp_pac));

    if (sendto(
        conn, payload, ARP_FRAME_LEN, 0,
        (struct sockaddr *)&addr, sizeof(struct sockaddr_ll)
    ) == -1) {
        perror("Couldn't broadcast arp");
    }

    free(payload);
}

void recv_frame(int conn){
    uint8_t * payload = malloc(ARP_FRAME_LEN);
    if(recv(conn, payload, ARP_FRAME_LEN, 0) == -1){
        perror("recv");
    }
    
    struct ethhdr header = *((struct ethhdr *)payload);

    if(header.h_proto != htons(ETH_P_ARP)){
        return;
    }

    payload += sizeof(struct ethhdr);
    
    struct arp_pac *reply = (struct arp_pac *)payload;
    char mac_str[20];
    char ip_str[16];

    uint8_t *ip = reply->spa;
    uint8_t *mac = reply->sha;

    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d",
         ip[0], ip[1], ip[2], ip[3]);

    printf("MAC: %s IP: %s\n", mac_str, ip_str);
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
