#include "minesweeperServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>


#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

#define MAXGAMESIZE 84
#define MAXDATASIZE 256

// Define what a tile is
typedef struct Tile{
	int adjacent_mines;
	bool revealed;
	bool is_mine;
}Tile;

typedef struct GameState {
	// More here
  int minesLeft;
	Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
}GameState;

void MinesweeeperMenu(int socket_id){
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
		printf("Sending gamestate\n");
		strcpy(gameString, FormatGameState(gamestate));
		for(int i = 0; i < MAXGAMESIZE+1; i++){
			printf("%c,", gameString[i]);
		}
		printf("\n");
		shortRetval = SendData(socket_id, gameString, MAXGAMESIZE);
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


char *FormatGameState(struct GameState gamestate){
	char gameString[MAXGAMESIZE];
	for(int x = 0; x < NUM_TILES_X; x++){
		for (int y = 0; y < NUM_TILES_Y; y++){
			int loc;

			loc = (x * NUM_TILES_X) + y;
			if(gamestate.tiles[x][y].revealed == true){
				strcpy(&gameString[loc], " ");
			} else{
				strcpy(&gameString[loc], " ");
			}
		}
	}
	if(gamestate.minesLeft == 10){
		strcpy(&gameString[82], "1");
		strcpy(&gameString[83], "0");
	}else {
		strcpy(&gameString[82], "0");
		char num[MAXDATASIZE];
		sprintf(num, "%d", gamestate.minesLeft);
		strcpy(&gameString[83], num);
	}

}

void SendMinesweeper(char gameString[MAXGAMESIZE], int socket_id){

}
