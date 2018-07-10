#include "helpers.h"
#include "parsers.h"
#include "sock_ops.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct ifreq ifr;

struct  __attribute__((packed)) arp_pac{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint8_t oper;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];
};

int main(int argc, char *argv[]) {
    
    if(argc < 2) { 
        printf("Enter network device name\n");
        exit(1);
    }

    int sock = make_socket(argv[1]);

    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
    if(ioctl(sock, SIOCGIFINDEX, &ifr)){
        perror("SIOCGIFINDEX");
        exit(1);
    }

    if(ioctl(sock, SIOCGIFHWADDR, &ifr)){
        perror("SIOCGIFHWADDR");
        exit(1);
    }
    
    if(ioctl(sock, SIOCGIFADDR, &ifr)){
        perror("SIOCGIFHWADDR");
        exit(1);
    }
    


    return 0;
}
