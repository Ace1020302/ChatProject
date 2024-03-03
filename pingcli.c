#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>

#define BUF_SIZE 500
#define USER_SIZE 25
#define MSG_SIZE 473

struct clientData {
	char message[MSG_SIZE];
	char username[USER_SIZE];
};

//struct sockaddr_in *createIPv4Address(char *ip, int port) {
//	
//	// Allocate a struct that contains necessary client data for connection
//	struct sockadrr_in *address = malloc(sizeof(struct sockaddr_in);
//	address.sin_family = AF_INET;
//	adress.sin_port = htons(port);
//
//	if(strlen(ip) == 0)
//
//}


void clientListen(int clientSocket) {
	char buf[BUF_SIZE];

	while (1)
	{
		ssize_t amountReceived = recv(clientSocket, buf, BUF_SIZE), 0);

		// buf contains propagated message from server
		// As long as there are characters in the received buf, write to chat
		if (amountReceived > 0)
		{
			buf[amountReceived] = 0;
			printf("%s\n", buf);
		}
		else 
			break;
	}

	close(clientSocket);
}

void startListenThread(int clientSocket) {
	pthread_t id;
	pthread_create(&id, NULL, clientListen, clientSocket);
}

void clientWrite(int clientSocket, char *user) {
 	char* msg[MSG_SIZE];
	printf("Client write here: \n");
	fgets(msg, MSG_SIZE, stdin);
	char* payload = strcat(strcat(user, ": "), msg);
	write(clientSocket, payload, strlen(payload));
}




int main(int argc, char *argv[])
{
	int bytes;
	char buf[BUF_SIZE];
	struct clientData cd;
	size_t len;
	ssize_t nread;

	struct sockaddr_in server_address;
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	int status = 0;
	int PORT_NUM = 3500; 

	//int returnCode = system("echo Hello World");

	if(argv[2] != NULL)
	{
		strcpy(cd.username, argv[2]);
	}
	else
	{
		strcpy(cd.username, "guest");
	}

	if (socket < 0)
	{
		perror("Socket creation failure");
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT_NUM);
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_zero[8] = '\0';

	status = connect(clientSocket, (struct sockaddr*)&server_address, sizeof(server_address));

	if (status < 0)
	{
		perror("Couldn't connect to server.");
		exit(EXIT_FAILURE);
	}

	startListenThread(clientSocket);
	
	while(1) {
		clientWrite(clientSocket, cd.username); 
	}

	close(clientSocket);

	return 0;
}
