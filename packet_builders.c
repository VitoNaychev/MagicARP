#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "packet_builders.h"
#include "parsers.h"

#define SA_DATA_LEN 14

struct arp_pac build_arp_request(uint32_t spa, uint32_t tpa, uint8_t *hwaddr){
    struct arp_pac packet = {
        .htype = htons(ARPHRD_ETHER),
        .ptype = htons(ETHERTYPE_IP),
        .hlen = ETH_ALEN,
        .plen = 4,
        .oper = htons(ARPOP_REQUEST)
    };

    spa = htonl(spa);
    tpa = htonl(tpa);

    memcpy(hwaddr, hwaddr, ETH_ALEN);

    memcpy(packet.sha, hwaddr, ETH_ALEN);
    memcpy(packet.spa, &spa, sizeof(uint8_t) * 4);

    memset(packet.tha, 0, ETH_ALEN);
    memcpy(packet.tpa, &tpa, sizeof(uint8_t) * 4);
    
    return packet;
}

uint8_t *get_if_hwaddr(int sock, char* if_name){
    struct ifreq ifr;
    
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if(ioctl(sock, SIOCGIFHWADDR, &ifr)){
        perror("SIOCGIFHWADDR");
        exit(1);
    }
    
    uint8_t* sa_data = malloc(SA_DATA_LEN);
    memcpy(sa_data, ifr.ifr_hwaddr.sa_data, SA_DATA_LEN);

    return sa_data;
}

int get_if_index(int sock, char* if_name){
    struct ifreq ifr;
    
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if(ioctl(sock, SIOCGIFINDEX, &ifr)){
        perror("SIOCGIFINDEX");
        exit(1);
    }

    return ifr.ifr_ifindex;
}

uint32_t get_if_netmask(int sock, char* if_name){
    struct ifreq ifr;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if(ioctl(sock, SIOCGIFNETMASK, &ifr)){
        perror("SIOCGIFNETMASK");
        exit(1);
    }

    uint32_t netmask = ((struct sockaddr_in*)
                 &ifr.ifr_netmask)->sin_addr.s_addr;
    return htonl(netmask);
}

uint32_t get_if_addr(int sock, char* if_name){
    struct ifreq ifr;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if(ioctl(sock, SIOCGIFADDR, &ifr)){
        perror("SIOCGIFNETADDR");
        exit(1);
    }

    uint32_t ip_addr = ((struct sockaddr_in*)
                 &ifr.ifr_netmask)->sin_addr.s_addr;
    
    return htonl(ip_addr);
}
