CC = gcc
CFLAGS = -g -Wall -Wpedantic -std=c17
LDFLAGS =
LDLIBS =

.PHONY: default
default: http-client

http-client: http-client.c

.PHONY: clean
clean:
	rm -rf a.out *.o http-client
