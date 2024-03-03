#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500
#define SERVER_PORT "8888"
#define SERVER_IP "127.0.0.1"

int main(int argc, char *argv[]) {
    int sfd, s, bytes;
    char buf[BUF_SIZE];
    char message[BUF_SIZE];
    ssize_t nread;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;/* TCP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    while (1) {
        printf("Enter message: ");
        fgets(message, BUF_SIZE, stdin);

        if (send(sfd, message, strlen(message), 0) == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }

        if (strcmp(message, "quit\n") == 0) {
            printf("Closing connection...\n");
            break;
        }

        memset(buf, 0, BUF_SIZE);
        if (recv(sfd, buf, BUF_SIZE, 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        printf("Message from server: %s\n", buf);
    }

    close(sfd);

    return 0;
}
