#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 500
#define PORT_NUM 8888
#define DISCONNECT_KEYWORD "quit\n"

/* Function to handle communication with a client */
void handle_client(int client_socket) {
    char buf[BUF_SIZE];
    ssize_t nread;

    while (1) {
        memset(buf, 0, BUF_SIZE);
        if ((nread = recv(client_socket, buf, BUF_SIZE, 0)) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        printf("Received message: %s", buf);
        if (strcmp(buf, DISCONNECT_KEYWORD) == 0) { /* Use the disconnect keyword here */
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

int main() {
    int sfd, s;
    struct sockaddr_in server_address, client_address;
    socklen_t client_addr_len;

    /* Create a socket */
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Set up server address */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket to server address */
    if (bind(sfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    /* Listen for incoming connections */
    if (listen(sfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT_NUM);

    while (1) {
        int client_socket;
        client_addr_len = sizeof(client_address);
        client_socket = accept(sfd, (struct sockaddr*)&client_address, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            close(client_socket);
            continue;
        }

        if (pid == 0) { /* Child process */
            close(sfd); /* Close listening socket in child process */
            handle_client(client_socket); /* Function to handle client communication */
            close(client_socket); /* Close client socket in child process */
            exit(EXIT_SUCCESS); /* Exit child process */
        } else { /* Parent process */
            close(client_socket); /* Close client socket in parent process */
        }
    }

    close(sfd);

    return 0;
}
