all: server proxy

CC=gcc

server: server.c server_func.c
	$(CC) -o $@ $^

proxy: proxy.c
	$(CC) -o $@ $<