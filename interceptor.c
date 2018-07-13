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
static int err_socket(int);
static void listen_socket(int);
static void connect_socket(int, struct sockaddr_in *);
static void send_all(int, uint8_t *, size_t);
static void print_buf(uint8_t *, size_t);

#define BUFF_SIZE 600

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
    ssize_t rread;

    // if (inet_pton(AF_INET, "0.0.0.0", &pc_addr.sin_addr) == -1) {
    //     perror("bad ip");
    //     exit(0);
    // }

    conn[0] = err_socket(1);  // from victim
    conn[1] = err_socket(1);  // to destination

    bind_socket(conn[0], &pc_addr);
    listen_socket(conn[0]);

    if ((victim = accept(conn[0], (struct sockaddr *)&victim_addr, &size)) == -1) {
        perror("accepting failed");
        exit(0);
    }

    size = sizeof(struct sockaddr_in); // reset if changed
    if (getsockopt(conn[0], SOL_IP, IP_ORIGDSTADDR, &dst_addr, &size)) {
        perror("couldn't get socket options");
        exit(0);
    }

    bind_socket(conn[1], &victim_addr);

    if (inet_pton(AF_INET, "194.153.145.104", &dst_addr.sin_addr) == -1) {
        perror("bad ip");
        exit(0);
    }
    dst_addr.sin_port = htons(80);

    connect_socket(conn[1], &dst_addr);

    // diagnostics
    inet_ntop(AF_INET, (const void *)&victim_addr.sin_addr, ip, INET_ADDRSTRLEN);
    printf("connected peer: %s\n", ip);
    inet_ntop(AF_INET, (const void *)&dst_addr.sin_addr, ip, INET_ADDRSTRLEN);
    printf("original destination: %s, port: %d\n", ip, ntohs(dst_addr.sin_port));
    inet_ntop(AF_INET, (const void *)&pc_addr.sin_addr, ip, INET_ADDRSTRLEN);
    printf("me me me: %s\n", ip);

    // it's http, i know the order; do porperly later
    while ((rread = recv(victim, buff, BUFF_SIZE, 0)) != 0) {
        if (rread == -1) {
            perror("fml");
            break;
        } else if (rread == 0) {
            // cannot possibly happen
            printf("hmmm\n");
            exit(0);
        }


        print_buf(buff, rread);
        send_all(conn[1], buff, rread);
    }

    while ((rread = recv(conn[1], buff, BUFF_SIZE, 0)) != 0) {
        if (rread == -1) {
            perror("fml");
            break;
        }

        print_buf(buff, rread);
        send_all(victim, buff, rread);
    }
    printf("\n");

    free(buff);

    return NULL;
}

static int err_socket(int set) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;

    if (sock == -1) {
        perror("Couldn't get socket");
        exit(0);
    }

    if (!set)
        return sock;

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

static void send_all(int sock, uint8_t * buf, size_t len) {
    size_t sent = 0;
    printf("len: %ld\n", len);

    while (len > 0) {
        ssize_t ssent = send(sock, buf + sent, len, MSG_NOSIGNAL);

        if (ssent == -1) {
            perror("sendall failed");
            exit(0);
        }

        len -= ssent;
        printf("remaining: %ld\n", len);
        sent += ssent;
    }

    printf("sent: %ld\n", sent);
}

static void print_buf(uint8_t * buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
}
