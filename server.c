#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include <time.h>
#include <unistd.h>

#include "minesweeperServer.h"

#define RANDOM_NUM_SEED 42

#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

#define TOTAL_CONNECTIONS 10
#define MAXDATASIZE 256

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

void ClientConnectionsHandler(int);
void PlaceMines();
void HandleExitSignal();
void ClientCommunicationHandler(int, char *[256]);

// Setup server, client socket variables
int serverListen, clientConnect, portNum;
struct sockaddr_in serv_addr, client;
socklen_t sin_size;

// Setup pthread variables
pthread_t client_thread;
pthread_attr_t attr;

int main(int argc, char* argv[]) {
	// Setup Handle Exit Signal
	signal(SIGINT, HandleExitSignal);

	// Seed the random number
	srand(RANDOM_NUM_SEED);

	// Handle Port Connection
	if (argc < 2) {
		fprintf(stderr, "%s\n", "No Port Provided - using default 12345");
		portNum = 12345;
	} else {
		portNum = atoi(argv[1]);
		printf("Port Provided - using %d\n", portNum);
	}

	// Generate server socket
	if ((serverListen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	printf("Generated server socket\n");

	// Setup Server Address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNum);

	// Bind config to server
	if ((bind(serverListen, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) == -1) {
		perror("bind");
		exit(1);
	}
	printf("Bound configuration to server\n");

	// Start listening
	if ((listen(serverListen, TOTAL_CONNECTIONS)) == -1) {
		perror("listen");
		exit(1);
	}
	printf("Server started listnening...\n");

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if((clientConnect = accept(serverListen, (struct sockaddr *) &client, &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("Server: received connection from %s\n", inet_ntoa(client.sin_addr));

		// Create a thread to accept client	
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&client_thread, &attr, ClientConnectionsHandler, clientConnect);

		pthread_join(client_thread, NULL);
	}

	close(clientConnect);
}

// TODO: DOESNT WORK AS EXPECTED
void HandleExitSignal() {
	// Close socket connection
	printf("\n\nClosing server and client sockets\n");
	shutdown(clientConnect, 1);
	shutdown(serverListen, 1);
	close (clientConnect);
	close (serverListen);

	// Kill threads and exit program
	printf("Killing threads, exiting program...\n");
	pthread_exit(&client_thread);

	// Exit program
	exit(0);
}

// Handle client connections
void ClientConnectionsHandler(int socket_id) {
	char message[MAXDATASIZE];
	int read_size;

	// Receive username
	read_size = ReceiveData(socket_id, message, MAXDATASIZE);
	char username[strlen(message)];
	strcpy(username, message);
	fprintf(stderr, "Received username: %s\n", username);

	// Receive password
	read_size = ReceiveData(socket_id, message, MAXDATASIZE);
	char password[strlen(message)];
	strcpy(password, message);
	fprintf(stderr, "Received password: %s\n", password);

	// Check authentication of user and pass and handle :)
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
