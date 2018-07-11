#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

static void make_sockets(int[2], char *);


void * interceptor(void * arg) {
    int conn[2];
    char * ip_addr; // TODO: set
    struct in_addr victim_addr;

    if (inet_pton(AF_INET, ip_addr, &victim_addr) == -1) {
        perror("bad ip");
        exit(0);
    }

    make_sockets(conn, ip_addr);

    while (1) {
        struct sockaddr_in peer;
        socklen_t size = sizeof(struct sockaddr_in);

        if (accepacceptt(conn[0], (struct sockaddr *)&peer, &size) == -1) {
            perror("accepting failed");
            exit(0);
        }

        if (peer.sin_addr.s_addr != victim_addr.s_addr) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, (const void *)&peer.sin_addr, ip, INET_ADDRSTRLEN);

            printf("WARNING: connection from wrong peer %s", ip);
        }

        struct sockaddr_in ignorant_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(80),  // intercept plain HTTP for now
        };

        if (connect(conn[1], )) {
            perror("failed to connect");
            pthread_exit(NULL);
        }
    }


    return NULL;
}

int err_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Couldn't get socket");
        exit(0);
    }

    return sock;
}

static void make_sockets(int conns[2], char * ip_addr) {
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(80),  // intercept plain HTTP for now
    };

    if (inet_pton(AF_INET, ip_addr, &addr.sin_addr) == -1) {
        perror("bad ip");
        exit(0);
    }

    conns[0] = err_socket();  // listening
    conns[1] = err_socket();  // writing

    if (bind(conns[0], (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind failed");
        exit(0);
    }

    if (listen(conns[0], 5)) { // for waiting connections for now
        perror("connecting failed");
        exit(0);
    }
}
