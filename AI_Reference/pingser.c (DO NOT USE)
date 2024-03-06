#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500
#define PORT "8888"

int main(int argc, char *argv[]) {
    int reuse_port = 1;
    int sfd, s, bytes;
    char buf[BUF_SIZE];
    ssize_t nread;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;/* TCP protocol socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(NULL, PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) < 0) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (listen(sfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s...\n", PORT);

    while (1) {
        int client_socket;
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        client_socket = accept(sfd, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        while (1) {
            memset(buf, 0, BUF_SIZE);
            if ((nread = recv(client_socket, buf, BUF_SIZE, 0)) == -1) {
                perror("recv");
                exit(EXIT_FAILURE);
            }
            printf("Received message: %s", buf);
            if (strcmp(buf, "quit\n") == 0) {
                printf("Client disconnected.\n");
                close(client_socket);
                break;
            }
            if (send(client_socket, buf, nread, 0) == -1) {
                perror("send");
                exit(EXIT_FAILURE);
            }
        }
    }

    close(sfd);

    return 0;
}
