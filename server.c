#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include <string.h>
#include <inttypes.h>
#include <malloc.h>

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

void* ClientConnectionsHandler(void *);
void PlaceMines();
void HandleExitSignal();
void ClientCommunicationHandler(int, char *[256]);
int NumAuths(char *);

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
		// TODO: Threading does not work with multiple concurrent users.
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&client_thread, &attr, ClientConnectionsHandler, (void *) clientConnect);

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
void* ClientConnectionsHandler(void *args) {
	int socket_id = (uintptr_t)args;
	char authFile[50] = "Authentication.txt";
	char loginDetails[13][2][256];

	FILE *fd;
	fd = fopen(authFile, "r");
	int x, y;
	for (x = 0; x < 12; x++){
		for(y = 0; y < 2; y++){
			fscanf(fd, "%s", loginDetails[x][y]);
		}
	}
	fclose(fd);

	char message[MAXDATASIZE], loginMessage[MAXDATASIZE];
	int read_size;
	while(strcmp(loginMessage, "1")!=0){
		// Receive username
		int user = -1;
		strcpy(loginMessage, "0");

		read_size = ReceiveData(socket_id, message, MAXDATASIZE);
		char username[strlen(message)];

		strcpy(username, message);
		fprintf(stderr, "Received username: %s\n", username);
		for(int i = 1; i < 12; i++){
			if(strcmp(username, loginDetails[i][0]) == 0){
				user = i;
			}
		}

		// Receive password
		read_size = ReceiveData(socket_id, message, MAXDATASIZE);
		char password[strlen(message)];
		strcpy(password, message);
		fprintf(stderr, "Received password: %s\n", password);
		int resMes = -1;
		if((user > 0) && (strcmp(password, loginDetails[user][1]) == 0)){
			// Continue
			printf("Correct Username and Password\n");
			strcpy(message, "1");
			strcpy(loginMessage, "1");

		}else{
			printf("Wrong Username or Password\n");
			strcpy(message, "0");
			strcpy(loginMessage, "0");
		}
		resMes = SendData(socket_id, message, MAXDATASIZE);
		printf("%s\n", loginMessage);
	}

	// Await information on what the client wishes to do
	printf("Awaiting instruction from user\n");
	int clientFinished = 0;
	while(clientFinished != 1){
		//char* res[MAXDATASIZE];
		int msg = ReceiveData(socket_id, message, MAXDATASIZE);
		//res = message;
		if (strcmp("1", message) == 0){
	    // Start Minesweeper


	  } else if (strcmp("2", message) == 0){
	    // Show Leaderboard


	  } else if (strcmp("3", message) == 0){
	    // Quit
			printf("Client Disconnecting\n");
	    close(socket_id);
			pthread_join(client_thread, NULL);
			clientFinished = 1;
	  }


	}

	// Generate Game State - TODO: Expand for multithreading
	//struct GameState gameState1;


}

// Place mines
void PlaceMines(){
	for (int i = 0; i < NUM_MINES; i++) {
		int x, y;

		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (TileContainsMine(x,y));

	}
}

void TileContainsMine(int x, int y) {

}
