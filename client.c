#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT_NUM 3500
#define BUF_SIZE 500

int main()
{
	struct sockaddr_in server_address;
	char msg[BUF_SIZE];
	char buf[BUF_SIZE];
	int s = socket(AF_INET, SOCK_STREAM, 0);
	int status = 0;

	if(s < 0)
	{
		perror("Socket creation failure");
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_zero[8] = '\0';

	status = connect(s, (struct sockaddr*)&server_address, sizeof(server_address));

	if(status < 0)
	{
		perror("Couldn't connect to the server");
		exit(EXIT_FAILURE);
	}

	while(1)//!(strcmp(msg, "exit")))
	{
		printf("Message: \n");
		fgets(msg, BUF_SIZE, stdin);
		//msg[BUF_SIZE - 1] = '\0';
		//send(s, msg, strlen(msg), 0);
		//read(s, buf, 100);
		printf("Message from client: %s\n", msg);
	}

	close(s);

	return 0;
}
