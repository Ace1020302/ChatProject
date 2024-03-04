all: server client

old: server_old client_old
	
ref: server_ref client_ref

server_ref: server_testing/stuserver.c
	gcc -o server server_testing/stuserver.c

client_ref: server_testing/stuclient.c
	gcc -o client server_testing/stuclient.c

server_old: server.c	
	gcc -o server server.c

client_old: client.c
	gcc -o client client.c

server: server.c
	gcc -o server pingser.c

client: client.c
	gcc -o client pingcli.c

clean:
	rm server
	rm client
