#ifndef PARSERS_H
#define PARSERS_H

#include <stdint.h>

struct arp_body {
    uint8_t src_ha[6];
    uint8_t src_pa[4];
    uint8_t dst_ha[6];
    uint8_t dst_pa[4];
};

void parse_arp(uint8_t *);
void parse_mac(uint8_t *);

#endif
