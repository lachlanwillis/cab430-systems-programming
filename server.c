#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include <string.h>

#include <netinet/in.h>

#include <signal.h>

#include <time.h>
#include <unistd.h>

#define RANDOM_NUM_SEED 42

#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

// Define what a tile is
typedef struct {
	int adjacent_mines;
	bool revealed;
	bool is_mine;
} Tile;

typedef struct {
	// More here
	Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
} GameState;

void *ClientConnectionsHandler(void *);
void PlaceMines();
void HandleExitSignal();
void signal_handler(int signal);
void *ClientCommunicationHandler(int, char *[256]);

// Setup server, client socket variables
int serverListen, clientConnect;

// Setup pthread variables
pthread_t tid;
pthread_attr_t attr;

int main(int argc, char* argv[]) {
	int portNum;
	struct sockaddr_in serv_addr, client;
	int c = sizeof(struct sockaddr_in);

	signal(SIGINT, HandleExitSignal);

	// Random Number
	srand(RANDOM_NUM_SEED);

	// Handle Port Connection
	if (argc < 2) {
		fprintf(stderr, "%s\n", "No Port Provided - using default 12345");
		portNum = 12345;
	} else {
		portNum = atoi(argv[1]);
		printf("Port Provided - using %d", portNum);
	}

	// Setup server socket
	serverListen = socket(AF_INET, SOCK_STREAM, 0);

	// Setup Server Address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNum);

	// Bind config to server
	bind(serverListen, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	// Listen for X connections - currently 10
	listen(serverListen, 10);
	
	// Attempt a connection
	while ((clientConnect = accept(serverListen, (struct sockaddr *) &client, (socklen_t*)&c))) {
		puts("Accepting connection");
		if (pthread_create(&tid, NULL, ClientConnectionsHandler, (void * __restrict__) & clientConnect) < 0){
			return 1;
		}
		pthread_join(tid, NULL);
		puts("Closing thread");
	}

	return 0;
}

// Handles Exiting on CTRL-C
void HandleExitSignal() {
	// Close socket connection
	printf("\n\nClosing server and client sockets\n");
	close (clientConnect);
	close (serverListen);

	// Kill threads and exit program
	printf("Killing threads, exiting program...\n");
	pthread_exit(&tid);

	// Exit program
	exit(0);
}

// Handle client connections
void *ClientConnectionsHandler(void *serverListen) {
	// Prepare writing to client
	char message[256] = "Successfully connected to server";

	puts("Successfully created thread");
	int socket = *(int*) serverListen;
	puts("Writing to client");
	ClientCommunicationHandler(socket, (char **)message);

	return 0;
}

// Handle sending data to client
void *ClientCommunicationHandler(int socket, char *message[256]) {
	write(socket, message, strlen(*message) + 1);

	return 0;
}

// Place mines
void PlaceMines(){
	for (int i = 0; i < NUM_MINES; i++) {
		int x, y;
		/*
		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (TileContainsMine(x,y));
		*/
	}
}

void TileContainsMine(int x, int y) {

}
