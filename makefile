all: server proxy client

CC=gcc

server: server.c server_func.c
	$(CC) -o $@ $^

proxy: proxy.c
	$(CC) -o $@ $^

client: client.c
	$(CC) -o $@ $^


