#include "minesweeperServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

#define RANDOM_NUM_SEED 42

#define MAXGAMESIZE 83
#define TOTAL_CONNECTIONS 10
#define MAXDATASIZE 256

// Define what a tile is
typedef struct Tile{
	bool flag_placed;
	int adjacent_mines;
	bool revealed;
	bool is_mine;
}Tile;

struct GameState {
	// More here

	bool GameOver;
  int minesLeft;
	Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
}GameState;


void MinesweeeperMenu(int socket_id){
	// Seed the random number
	srand(RANDOM_NUM_SEED);
	// Create GameState
  struct GameState gamestate;
	printf("Placing Mines\n");
  gamestate = PlaceMines();
  gamestate.minesLeft = NUM_MINES;
	printf("Mines placed\n");
	int playing = 1;

	while(playing){
		int shortRetval = -1;
		char gameString[MAXGAMESIZE];
		char ret[MAXDATASIZE];
		char clientReq[MAXDATASIZE];

		int res = 1;

		// Wait for client to request gameState
		while(shortRetval = ReceiveData(socket_id, clientReq, MAXDATASIZE) > 0){
			if (strncmp(clientReq, "1", 1) != 0){
				printf("Error with request from client\n");
			}

		}

		printf("Sending gamestate\n");
		FormatGameState(gamestate, gameString);
		for(int i = 0; i < MAXGAMESIZE+1; i++){
			printf("%c,", gameString[i]);
		}

		printf("\n");
		shortRetval = SendData(socket_id, gameString, MAXGAMESIZE+1);

    playing = 0;
    char chosenOption[8];
    shortRetval = ReceiveData(socket_id, chosenOption, 8);
    printf("Received Data\n");
    for(int i = 0; i < 8; i++){
      printf("%s", &chosenOption[i]);
    }
    printf("\n");

		// Convert String provided to int
		int coords = strtol(&chosenOption[1], NULL, 10);
    if (strncmp(&chosenOption[0], "r", 1) == 0){
      // Flip Tile
      printf("User chose to Flip Tile\n");
			FlipTile(&gamestate, coords);
			playing = 1;
    } else if (strncmp(&chosenOption[0], "p", 1) == 0){
      // Place Flag
      printf("User chose to Place Flag\n");
    } else if (strncmp(&chosenOption[0], "q", 1) == 0){
      // User chose to quit
      playing = 0;
      printf("User chose to quit\n");
			return;
    } else{
      printf("Error with string\n");
    }
	}
}

// create functions here that are defined in the header
int ReceiveData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = recv(serverSocket, message, messageSize, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  message[shortRetval]='\0';
  return shortRetval;
}

int SendData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = send(serverSocket, message, messageSize, 0)) == -1) {
    perror("send");
		exit(1);
  }
  return shortRetval;
}


int TileContainsMine(int x, int y, struct GameState gamestate) {
	if(gamestate.tiles[x][y].is_mine){
    return 1;
  } else {
    return 0;
  }
}

// Place mines
struct GameState PlaceMines(){
  struct GameState gamestate;
  for(int x_tile = 0; x_tile < NUM_TILES_X; x_tile++){
    for(int y_tile = 0; y_tile < NUM_TILES_Y; y_tile++){
      gamestate.tiles[x_tile][y_tile].revealed = false;
			gamestate.tiles[x_tile][y_tile].adjacent_mines = 0;
    }
  }

	for (int i = 0; i < NUM_MINES; i++) {
		int x, y;
		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (TileContainsMine(x,y, gamestate));
    gamestate.tiles[x][y].is_mine = true;
		printf("Placing mine at %d/%d\n", x, y);
		for(int a = -1; a < 2; a++){
			for (int b = -1; b < 2; b++){
				if((a + x > -1) && (a + x < NUM_TILES_X)){
					if((b + y > -1) && (b + y < NUM_TILES_Y)){
						if(a!=x && b!=y){
							gamestate.tiles[x][y].adjacent_mines++;
						}
					}
				}
			}
		}
	}
  return gamestate;
}


// Prepare the gamestate into an easy to communicate gameString
void FormatGameState(struct GameState gamestate, char* gameString){
	// initialise the string to be empty
	for(int initial = 0; initial < MAXGAMESIZE; initial++){
		gameString[initial] = ' ';
	}
	// Loop through each element and set it to the appropriate value
	for(int x = 0; x < NUM_TILES_X; x++){
		for (int y = 0; y < NUM_TILES_Y; y++){
			int loc;
			loc = x + (y * NUM_TILES_Y);

      if(gamestate.tiles[x][y].revealed == true){
				printf("Tile %d/%d is revealed\n", x, y);
        gameString[loc] = gamestate.tiles[x][y].adjacent_mines;

			}else{
        gameString[loc] = ' ';

			}
		}
	}

	if(gamestate.minesLeft == 10){
		gameString[82] = '1';
		gameString[83] = '0';
	}else {
		gameString[82] = ' ';
		gameString[83] = gamestate.minesLeft;
	}
}


void SendLeaderboard(int socket, struct LeaderboardEntry *leaderboard) {
	int time_count, won, played;

	for (int i = 0; i < TOTAL_CONNECTIONS; i++) {
		time_count = htonl(leaderboard[i].time);
		won = htonl(leaderboard[i].won);
		played = htonl(leaderboard[i].played);

		SendData(socket, leaderboard[i].username, MAXDATASIZE);
		write(socket, &time_count, sizeof(time_count));
		write(socket, &won, sizeof(won));
		write(socket, &played, sizeof(played));
	}
}


void FlipTile(struct GameState *gameState, int loc){
	int x_tile, y_tile;
	x_tile = loc/10;
	y_tile = loc - x_tile*10;
	// Check to see if tile is a mine
	if( (*gameState).tiles[x_tile][y_tile].is_mine == true){
		// Game Over
		(*gameState).GameOver = true;
	} else {
		// If tile is not a mine - flip the tile, to reveal number below
		(*gameState).tiles[x_tile][y_tile].revealed = true;
		printf("Flipped Tile %d/%d\n", x_tile,y_tile);
	}


	// If tile is a 0, flip the tiles around it. Repeat (Most likely call FlipTile for the tiles around)

}



void FlagTile(struct GameState *gameState, int loc){


}

void GameOverMsg(int time, int won){
	if (won){
		// Send Congrats

	} else {
		// Send Gameover
	}

}
