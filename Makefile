CC = gcc
LDFLAGS = -pthread

# LIST ALL OUTPUT FILES WE WANT
all: client server

# CLIENT FILES
client: minesweeperClient.o client.o

minesweeperClient.o: minesweeperClient.c

client.o: client.c

# SERVER FILES
server: minesweeperServer.o server.o

minesweeperServer.o: minesweeperServer.c

server.o: server.c

# CLEAN
clean:
	rm -f client
	rm -f server
	rm -f *.o

.PHONY: all clean
