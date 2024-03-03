#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 5
#define BUF_SIZE 1024
#define PORT_NUM 3500
static int client_number = 0;
static int num_of_clients = 0;
static int connected = 0;
//int sfd; // Socket File Descriptor

struct client_struct{
	struct sockaddr_in address;
	int socket_descriptor;
	int client_id;
};

struct client_struct *clients[MAX_CLIENTS];

// https://linux.die.net/man/3/pthread_mutex_lock
// https://stackoverflow.com/questions/14320041/pthread-mutex-initializer-vs-pthread-mutex-init-mutex-param
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void enqueue_client(struct client_struct *client){
	pthread_mutex_lock(&lock);

	for(int i = 0; i <= MAX_CLIENTS; ++i){
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

	clients[client_id] = NULL;
	num_of_clients--;

	pthread_mutex_unlock(&lock);
}

/* Sends the message to all clients except the sending client */
void message(char *msg, int sending_client_id){
	pthread_mutex_lock(&lock);

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(clients[i] && clients[i]->socket_descriptor != sending_client_id)
		{
			send(clients[i]->socket_descriptor, msg, strlen(msg), 0);
		}
	}

	pthread_mutex_unlock(&lock);
}

void *manage_connections(int sfd){
	char buf[BUF_SIZE];
	ssize_t amount_rec;
	//int connected = 0;

	//struct client_struct *client = (struct client_struct *)arg;

	//message(buf, client->client_id);
	//printf("%s\n", buf);
	// https://www.man7.org/linux/man-pages/man3/memset.3.html
	memset(buf, 0, BUF_SIZE);

	while(1)
	{
		amount_rec = recv(sfd, buf, BUF_SIZE, 0);
		//buf[num_of_bytes] = '\0';
	
		/*
		if(amount_rec == 0) // We received no data, we need to break out and close
		{
			break;
		}
		*/
		buf[amount_rec] = 0;
		printf("%s\n", buf);
		//memset(buf, 0, BUF_SIZE);
		//send(client_socket, msg, strlen(msg), 0);


		if(strcasecmp(buf, "exit") == 0)
		{
			sprintf(buf, "%s has left the chat", sfd);
			printf("%s\n", buf);
			message(buf, sfd);
			close(sfd);
			dequeue_client(sfd);
			free(sfd);
			//break;
		}
		else if(amount_rec > 0)
		{
			buf[amount_rec] = 0;
			printf("PID: %d; server received %s\n", getpid(), buf);
			message(buf, sfd);
		}
		memset(buf, 0, BUF_SIZE);
	}
	//close(sfd);
	//dequeue_client(sfd);

	//free(sfd);

	pthread_detach(pthread_self());
}

struct client_struct *acceptNewConnection(int sfd)
{
	struct sockaddr_in clientAddress;
	int cliAddrSize = sizeof(struct sockaddr_in);
	int cliSFD = accept(sfd, &clientAddress, &cliAddrSize);

	struct client_struct* acceptedSocket = malloc(sizeof(struct client_struct));
	acceptedSocket->address = clientAddress;
	acceptedSocket->socket_descriptor = cliSFD;
	acceptedSocket->client_id = client_number++;

	return acceptedSocket;
}

int main(int argc, char *argv[])
{
	int connection;
	int reuse_port = 1;
	int socket_descriptor, client_number, listen_status, bind_status; 
	char buf[BUF_SIZE]; //Message buffer
	struct sockaddr_in server_address, connection_address;
	unsigned connected_clients[MAX_CLIENTS];
	pthread_t pid;

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0); // initialize server's socket
	if(socket_descriptor < 0) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	/*
	int status = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_port, sizeof(reuse_port));
	if(status < 0) {
		perror("Couldn't set options");
	}
	*/

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);
	server_address.sin_addr.s_addr = INADDR_ANY;
	//server_address.sin_zero[8] = '\0';
	bind_status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));

	if(bind_status < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	listen_status = listen(socket_descriptor, MAX_CLIENTS); //Max of 5 clients

	int length_addr = sizeof(connection_address);

	//connected_clients[num_of_clients++] = sfd;

	/*
	if (sfd < 0) {
		perror("Couldn't establish connection to client");
		exit(EXIT_FAILURE);
	}
	*/

	printf("CSCI 3160 - Stupid Discord Server:\n");

	while(!(connected)){
		struct client_struct* client = acceptNewConnection(socket_descriptor);
		//client->address = connection_address;

		enqueue_client(client);
		// https://www.man7.org/linux/man-pages/man3/pthread_create.3.html
		pthread_create(&pid, NULL, manage_connections, client->socket_descriptor);
	}

	return EXIT_SUCCESS;
}
