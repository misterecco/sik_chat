CC = gcc
CFLAGS = -Wall -O2
TARGETS = client server

all: $(TARGETS)

err.o: err.c err.h

client.o: client.c err.h

client: client.o err.o

server.o: server.c err.h

server: server.o err.h

clean:
	rm -f *.o $(TARGETS)
