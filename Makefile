CC = gcc
CFLAGS = -Wall -O2 -DDEBUG
TARGETS = client server

all: $(TARGETS)

err.o: err.c err.h

common.o: common.c common.h err.h

client.o: client.c err.h common.h

client: client.o err.o common.o

server.o: server.c err.h common.h

server: server.o err.o common.o

clean:
	rm -f *.o $(TARGETS)
