#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>

int main()
{
  struct ifreq s;
  int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, "wlp2s0", strlen("wlps0"))){
    perror("nqob");
  }

  strcpy(s.ifr_name, "wlp2s0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
    for (i = 0; i < 6; ++i)
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    puts("\n");
    return 0;
  }else {
    perror("boqn");
  }
  return 1;
}


