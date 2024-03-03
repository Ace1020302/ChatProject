#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 500
#define PORT_NUM 8888
#define SERVER_IP "127.0.0.1"

int main() {
    struct sockaddr_in server_address;
    char msg[BUF_SIZE];
    char buf[BUF_SIZE];
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int status = 0;

    if (s < 0) {
        perror("Socket creation failure");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    status = connect(s, (struct sockaddr*)&server_address, sizeof(server_address));

    if (status < 0) {
        perror("Couldn't connect to the server");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter message: ");
        fgets(msg, BUF_SIZE, stdin);
        if (send(s, msg, strlen(msg), 0) == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        if (strcmp(msg, "quit\n") == 0) {
            printf("Closing connection...\n");
            break;
        }
        memset(buf, 0, BUF_SIZE);
        if (recv(s, buf, BUF_SIZE, 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        printf("Message from server: %s\n", buf);
    }

    close(s);

    return 0;
}
