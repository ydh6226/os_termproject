all: server proxy client

CC=gcc

server: server.c file_processing.c
	$(CC) -o $@ $^

proxy: proxy.c file_processing.c
	$(CC) -o $@ $^

client: client.c
	$(CC) -o $@ $^


