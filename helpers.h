#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

char * format_mac(uint8_t *);
uint16_t calc_ip_checksum(struct ip *);

#endif
