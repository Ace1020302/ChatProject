#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define MAX_CLIENTS 5
#define BUF_SIZE 500

static int num_of_clients = 0;
int sfd; // Socket File Descriptor

struct client_struct{
    struct sockaddr_in address;
    struct client_struct *next;
    struct client_struct *prev;

    int socket_descriptor;
    int client_id;
};

struct client_struct *clients[MAX_CLIENTS];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void enqueue_client(struct client_struct *client){
    pthread_mutex_lock(&lock);

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){
            clients[i] = client;
            clients[i]->client_id = i + 1;
            num_of_clients++;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

void dequeue_client(int client_id){
    pthread_mutex_lock(&lock);

    clients[client_id - 1] = NULL;
    num_of_clients--;

    pthread_mutex_unlock(&lock);
}

void message(char *msg, int sending_client_id){
    pthread_mutex_lock(&lock);

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(clients[i] && clients[i]->client_id != sending_client_id)
        {
            send(clients[i]->socket_descriptor, msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

void *manage_connections(void *arg){
    char buf[BUF_SIZE];

    struct client_struct *client = (struct client_struct *)arg;

    memset(buf, 0, BUF_SIZE);

    while(1)
    {
        int recvNum = recv(client->socket_descriptor, buf, BUF_SIZE, 0);

        if(recvNum > 0)
        {
            printf("Message from client %d: %s\n", client->client_id, buf);
            message(buf, client->client_id);

            if(strcasecmp(buf, "quit") == 0)
            {
                printf("Client %d has left the chat\n", client->client_id);
                message("You have left the chat.\n", client->client_id);
                break; // Exit the loop and thread
            }
        }
        else if(recvNum == 0)
        {
            printf("Client %d disconnected\n", client->client_id);
            break; // Exit the loop and thread
        }
        else
        {
            perror("Error receiving data from client");
            break; // Exit the loop and thread
        }

        memset(buf, 0, BUF_SIZE);
    }

    close(client->socket_descriptor);
    dequeue_client(client->client_id);
    free(client);

    pthread_detach(pthread_self());
}

int main(int argc, char *argv[])
{
    int socket_descriptor;
    struct sockaddr_in server_address, connection_address;
    pthread_t pid;

    int portNum = 3500;
    if(argv[1] != NULL)
    {
        portNum = atoi(argv[1]);
    }

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_descriptor < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNum);
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_zero[8] = '\0';
    int status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));

    if(status < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    status = listen(socket_descriptor, MAX_CLIENTS); //Max of 5 clients

    if (status < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("CSCI 3160 - Stupid Discord Server:\n");

    while(1){
        socklen_t length_addr = sizeof(connection_address);
        sfd = accept(socket_descriptor, (struct sockaddr*)&connection_address, &length_addr);

        if (sfd < 0) {
            perror("Couldn't establish connection to client");
            exit(EXIT_FAILURE);
        }

        struct client_struct *client = (struct client_struct *)malloc(sizeof(struct client_struct));
        client->address = connection_address;
        client->socket_descriptor = sfd;

        enqueue_client(client);
        pthread_create(&pid, NULL, &manage_connections, (void*)client);
    }

    close(socket_descriptor);

    return EXIT_SUCCESS;
}
