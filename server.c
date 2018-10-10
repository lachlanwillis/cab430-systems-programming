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
typedef struct{
	int adjacent_mines;
	bool revealed;
	bool is_mine;
} Tile;




typedef struct GameState{
	// More here
	Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
};



void *ClientConnectionsHandler(void *);
void PlaceMines();
void HandleExitSignal(int);
void *ClientCommunicationHandler(int, char *[256]);




int main(int argc, char* argv[]) {
	// Random Number
	srand(RANDOM_NUM_SEED);

	// Uncomment once we have implemented HandleExitSignal
	//signal(SIGINT, HandleExitSignal);

	// Setup pthread
	pthread_t tid;
	pthread_attr_t attr;

	// Handle Port Connection
	int portNum;
	if(argc < 2){
		fprintf(stderr, "%s\n", "No Port Provided - using default 12345");
		portNum = 12345;
	} else {
		portNum = atoi(argv[1]);
	}



	// Setup Server and client variables
	int serverListen, clientConnect;
	serverListen = socket(AF_INET, SOCK_STREAM, 0);

	// Setup Server Address
	struct sockaddr_in serv_addr, client;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNum);

	// Bind config to server
	bind(serverListen, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	int c = sizeof(struct sockaddr_in);

	// Listen for X connections - currently 10
	listen(serverListen, 10);
	// Attempt a connection
	while(clientConnect = accept(serverListen, (struct sockaddr *) &client, (socklen_t*)&c)){
		puts("Accepting connection");
		if (pthread_create(&tid, NULL,  ClientConnectionsHandler, (void * __restrict__) &clientConnect)< 0){
			return 1;
		}
		pthread_join(tid, NULL);
		puts("Closing thread");

	}
	// Attempt a connection

	// Send test data


	close (serverListen);




	return 0;

}





// Handle client connections
void *ClientConnectionsHandler(void *serverListen){

	// Prepare writing to client
	char message[256] = "Successfully connected to server";

	puts("Successfully created thread");
	int socket = *(int*) serverListen;
	puts("Writing to client");
	//write(socket, message, sizeof(message));
	ClientCommunicationHandler(socket, &message);




}


// Handle sending data to client
void *ClientCommunicationHandler(int socket, char *message[256]){
	write(socket, message, strlen(message) + 1);
	return 0;
}



// Place mines
void PlaceMines(){
	for (int i = 0; i < NUM_MINES; i++){
		int x, y;
		/*
		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (TileContainsMine(x,y));
		*/
	}
}



void TileContainsMine(int x, int y){

}





// Need to implement signal handler to exit cleanly when ctrl+c is pressed
void HandleExitSignal(int signal){
	// Exit code goes here.
}
