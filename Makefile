all: server client

server: server.c
	gcc -o server pingser.c

client: client.c
	gcc -o client pingcli.c

clean: server client
	rm server
	rm client
