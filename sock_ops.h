#ifndef SOCK_OPS_H
#define SOCK_OPS_H

#include <stdint.h>

int make_socket();
uint8_t * read_frame(int);
void set_promiscuous(int conn, char * if_name);

#endif
