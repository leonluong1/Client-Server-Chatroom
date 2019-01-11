main: client server

client: client.c
	gcc -std=c99 client.c -o client
server: server.c
	gcc -std=c99 server.c -o server
