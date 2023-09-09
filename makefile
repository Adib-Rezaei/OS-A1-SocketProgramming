all: server client

server: server.c
	gcc server.c -w -o server

client: client.c 
	gcc client.c -w -o client

.PHONY: clean

clean:
	rm client server