#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CLIENTS 5
#define BUF_SIZE 500
#define PORT_NUM 3500

int server_send();

int main(int argc, char *argv[])
{
	int connection;
	int reuse_port = 1;
	int socket_descriptor, client_socket; 
	char buf[BUF_SIZE]; //Message buffer
	int client_number;
	struct sockaddr_in server_address, connection_address;
	char* msg = "yuh";
	unsigned connected_clients[MAX_CLIENTS];
	int num_of_clients = 0;
	int i;
	int num_of_bytes;
	
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
	
	client_socket = accept(socket_descriptor, (struct sockaddr*)&connection_address, &length_addr);
	connected_clients[num_of_clients++] = client_socket;

	if (client_socket < 0) {
		perror("Couldn't establish connection to client");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		recv(client_socket, buf, BUF_SIZE, 0);
		//buf[num_of_bytes] = '\0';
		printf("Message from client: %s\n", buf);
		memset(buf, 0, BUF_SIZE);
		//send(client_socket, msg, strlen(msg), 0);
	}

	close(socket_descriptor);
	close(client_socket);
	return 0;
}

