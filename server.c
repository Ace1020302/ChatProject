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
#define PORT_NUM 3500
#define MAX_NAME_LENGTH 20

static int client_number = 0;
static int num_of_clients = 0;
static int connected = 0;


struct client_struct{
	struct sockaddr_in address;
	struct client_struct *next;
	struct client_struct *prev;

	int socket_descriptor;
	int client_id;
	char userName[MAX_NAME_LENGTH];
};

struct client_struct *clients[MAX_CLIENTS];

// https://linux.die.net/man/3/pthread_mutex_lock
// https://stackoverflow.com/questions/14320041/pthread-mutex-initializer-vs-pthread-mutex-init-mutex-param
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

	for(int i = client_id; i < num_of_clients; ++i)
	{
		if(i = MAX_CLIENTS) { break; }
		clients[i] = clients[i + 1];
	}
	num_of_clients--;

	pthread_mutex_unlock(&lock);
}

/* Sends the message to all clients except the sending client */
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
	int connected = 0;
	char uname_set[MAX_NAME_LENGTH];
	struct client_struct *client = (struct client_struct *)arg;

	// https://www.man7.org/linux/man-pages/man3/memset.3.html
	recv(client->socket_descriptor, uname_set, MAX_NAME_LENGTH, 0);
	strcpy(client->userName, uname_set);
	memset(buf, 0, BUF_SIZE);

	while(1)
	{
		int recvNum = recv(client->socket_descriptor, buf, BUF_SIZE, 0);

		if(recvNum > 0)
		{
			printf("PID: %d; server received %s\n", getpid(), buf);
			message(buf, client->client_id);
		}
		// Exit Case
		else if(strcasecmp(buf, "exit") == 0 || recvNum == 0)
		{
			sprintf(buf, "%s has left the chat", client->userName);
			printf("%s\n", buf);
			message(buf, client->client_id);
			break;
		}

		memset(buf, 0, BUF_SIZE);
	}
	// Close the client socket
	close(client->socket_descriptor);
	dequeue_client(client->client_id);

	free(client);

	pthread_detach(pthread_self());
}

int main(int argc, char *argv[])
{
	int connection;
	int reuse_port = 1;
	int socket_descriptor, client_number; 
	char buf[BUF_SIZE]; //Message buffer
	struct sockaddr_in server_address, connection_address;
	unsigned connected_clients[MAX_CLIENTS];
	int sfd;
	pthread_t pid;

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_descriptor < 0) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	int status = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_port, sizeof(reuse_port));
	if(status < 0) {
		perror("Couldn't set options");
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_zero[8] = '\0';
	status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));

	if(status < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	status = listen(socket_descriptor, MAX_CLIENTS); //Max of 5 clients

	int length_addr = sizeof(connection_address);

	printf("CSCI 3160 - Stupid Discord Server:\n");

	while(!(connected)){
		sfd = accept(socket_descriptor, (struct sockaddr*)&connection_address, &length_addr);
		struct client_struct *client = (struct client_struct *)malloc(sizeof(struct client_struct));
		client->address = connection_address;
		client->socket_descriptor = sfd;

		enqueue_client(client);
		// https://www.man7.org/linux/man-pages/man3/pthread_create.3.html
		pthread_create(&pid, NULL, &manage_connections, (void*)client);
	}

	return EXIT_SUCCESS;
}
