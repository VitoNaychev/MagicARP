#ifndef PARSERS_H
#define PARSERS_H

#include <stdint.h>

struct  __attribute__((packed)) arp_pac {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];
};

struct arp_body {
    uint8_t src_ha[6];
    uint8_t src_pa[4];
    uint8_t dst_ha[6];
    uint8_t dst_pa[4];
};

void parse_arp(uint8_t *);
void parse_mac(uint8_t *);

#endif
