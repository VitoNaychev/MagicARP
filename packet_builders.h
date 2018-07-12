#ifndef PACKET_BULDERS_H
#define PACKET_BUILDERS_H

#include <stdint.h>
#include "parsers.h"

struct arp_pac build_arp_request(uint32_t, uint32_t, uint8_t*);

uint8_t *get_if_hwaddr(int, char*);

int get_if_index(int, char*);

unsigned long get_if_netmask(int, char*);

#endif
