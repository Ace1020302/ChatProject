all: server client pingser pingcli

server: server.c
    gcc -o server server.c

client: client.c
    gcc -o client client.c

pingser: pingser.c
    gcc -o pingser pingser.c

pingcli: pingcli.c
    gcc -o pingcli pingcli.c

clean:
      rm -f server client pingser pingcli
