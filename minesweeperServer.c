#include "minesweeperServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>


#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

// Define what a tile is
typedef struct{
	int adjacent_mines;
	bool revealed;
	bool is_mine;
}Tile;

typedef struct GameState {
	// More here
  int minesLeft;
	Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
};

void MinesweeeperMenu(){
  struct GameState gamestate;
  gamestate = PlaceMines();
  gamestate.minesLeft = NUM_MINES;

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
    return 0;
  } else {
    return 1;
  }
}

// Place mines
struct GameState PlaceMines(){
  struct GameState gamestate;
	for (int i = 0; i < NUM_MINES; i++) {
		int x, y;
		do {
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		} while (TileContainsMine(x,y, gamestate));
    gamestate.tiles[x][y].is_mine = true;
	}
  return gamestate;
}
