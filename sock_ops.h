#ifndef SOCK_OPS_H
#define SOCK_OPS_H

#include "parsers.h"
#include <stdint.h>

int make_socket(char *);
uint8_t * read_frame(int);
void set_promiscuous(int conn, char * if_name);
uint8_t * broadcast_frame(int conn, struct arp_pac * data);

#endif
