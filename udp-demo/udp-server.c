#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define BUF_SIZE 512

/*
编译 gcc udp-server.c -o udp-server
运行 ./udp-server 50018
*/
int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd = -1, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t num_read;

    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* 允许IPv4 或者 IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* UDP */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    for (;;) {
        peer_addr_len = sizeof(struct sockaddr_storage);
        num_read = recvfrom(sfd, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&peer_addr, &peer_addr_len);
        buf[num_read] = '\0';

        if (num_read == -1)
            continue;

        char host[NI_MAXHOST], service[NI_MAXSERV];
        s = getnameinfo((struct sockaddr *)&peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0) {
            printf("receive %zd bytes: \"%s\" from %s:%s\n", num_read, buf, host, service);
        } else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        }

        sendto(sfd, buf, (size_t)num_read, 0,
               (struct sockaddr *) &peer_addr, peer_addr_len);
    }
}
