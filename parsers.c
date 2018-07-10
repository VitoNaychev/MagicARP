#include "helpers.h"
#include "parsers.h"

// socket libraries
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_arp.h>

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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

    free(src_mac);
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

    free(src);
    free(dest);
}
