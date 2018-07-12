#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <errno.h>

#include <linux/netfilter_ipv4.h>

static void bind_socket(int, struct sockaddr_in *);
static int err_socket();
static void listen_socket(int);
static void connect_socket(int, struct sockaddr_in *);

#define BUFF_SIZE 64

void * interceptor(void * arg) {
    // file descripors
    int conn[2];
    int victim;

    struct sockaddr_in victim_addr;
    struct sockaddr_in dst_addr;
    // this is the struct for _THIS_ program
    struct sockaddr_in pc_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),  // traffic gets forwarded to 8080
        .sin_addr.s_addr = htonl(INADDR_ANY)  // should equal 0.0.0.0
    };

    socklen_t size = sizeof(struct sockaddr_in);

    char ip[INET_ADDRSTRLEN];
    uint8_t * buff = malloc(BUFF_SIZE);
    int rread;

    // if (inet_pton(AF_INET, "0.0.0.0", &pc_addr.sin_addr) == -1) {
    //     perror("bad ip");
    //     exit(0);
    // }

    conn[0] = err_socket();  // from victim
    conn[1] = err_socket();  // to destination

    bind_socket(conn[0], &pc_addr);
    listen_socket(conn[0]);

    if ((victim = accept(conn[0], (struct sockaddr *)&victim_addr, &size)) == -1) {
        perror("accepting failed");
        exit(0);
    }

    size = sizeof(struct sockaddr_in); // reset if changed
    if (getsockopt(conn[0], SOL_IP, SO_ORIGINAL_DST, &dst_addr, &size)) {
        perror("couldn't get socket options");
        exit(0);
    }

    bind_socket(conn[1], &victim_addr);
    connect_socket(conn[1], &dst_addr);

    // diagnostics
    inet_ntop(AF_INET, (const void *)&victim_addr.sin_addr, ip, INET_ADDRSTRLEN);
    printf("connected peer: %s\n", ip);

    // it's http, i know the order; do porperly later
    while ((rread = recv(victim, buff, BUFF_SIZE, 0)) != 0) {
        if (rread == -1) {
            perror("fml");
            break;
        }
        send(conn[1], buff, BUFF_SIZE, 0);
        printf("%c", *buff);
    }
    printf("\n");

    while ((rread = recv(conn[1], buff, BUFF_SIZE, 0)) != 0) {
        if (rread == -1) {
            perror("fml");
            break;
        }
        send(victim, buff, BUFF_SIZE, 0);
        printf("%c", *buff);
    }
    printf("\n");

    free(buff);

    return NULL;
}

static int err_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;

    if (sock == -1) {
        perror("Couldn't get socket");
        exit(0);
    }

    if (setsockopt(sock, IPPROTO_IP, IP_TRANSPARENT, &flag, sizeof(int)) == -1) {
        perror("failed to make transparent");
        exit(0);
    }

    // if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
    //     perror("failed to make transparent");
    //     exit(0);
    // }

    return sock;
}

static void bind_socket(int sock, struct sockaddr_in * addr) {
    if (bind(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind failed");
        printf("%d\n", errno);
        exit(0);
    }
}

static void listen_socket(int sock) {
    // wait for no more than 5 conns
    if (listen(sock, 5)) {
        perror("listening failed");
        exit(0);
    }
}

static void connect_socket(int sock, struct sockaddr_in * addr) {
    if (connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
        perror("failed to connect");
        exit(0);
    }
}
