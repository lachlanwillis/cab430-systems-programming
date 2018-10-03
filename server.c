#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <pthreads.h>
#include <signal.h>

#include <unistd.h>


#define RANDOM_NUM_SEED 42;

#define NUM_TILES_X 9;
#define NUM_TILES_Y 9;
#define NUM_MINES 10;


// Define what a tile is
typedef struct{
	int adjacent_mines;
	bool revealed;
	bool is_mine;
} Tile;




typedef struct GameState{
	// More here
	Tile tiles[NUM_TILES_X, NUM_TILES_Y];
}



void *ClientConnectionsHandler(void *);




int main(int argc, char* argv[]) {
	// Random Number
	srand(RANDOM_NUM_SEED)

	signal(SIGINT, HandleExitSignal);
	// Handle Given Port Connection
	int portNum;
	if(argc < 1){
		fprintf(stderr, "%s\n", "ERROR, No String Provided!");
		exit(1);
	} else {
		portNum = atoi(argv[1]);
	}



	// Setup Server and client variables
	int serverListen = 0, clientConnect = 0;
	serverListen = socket(AF_INET, SOCK_STREAM, 0);

	// Setup Server Address
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = portNum;

	// Bind config to server
	bind(serverListen, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	// Listen for X connections - currently 10
	listen(serverListen, 10);


	// Client Connections Without Multithreading


}





// Handle client connections
void *ClientConnectionsHandler(void *serverListen){

}


// Place mines
void PlaceMines(){
	for (int i = 0; i < NUM_MINES; i++){
		int x, y;
		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (tile_contains_mine(x,y));

	}
}


// Need to implement signal handler to exit cleanly when ctrl+c is pressed
void HandleExitSignal(int signal){
	// Exit code goes here.
}
