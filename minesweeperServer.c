#include "minesweeperServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

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

char gameString[MAXGAMESIZE];
time_t start_time, end_time;

void MinesweeperMenu(int socket_id){
	// Start timer
	start_time = time(NULL);

	// Seed the random number
	srand(RANDOM_NUM_SEED);

	// Create GameState
  struct GameState gamestate;
	int shortRetval = -1, res, playing = 1;
	char gameString[MAXGAMESIZE], clientReq[MAXDATASIZE], chosenOption[8];

	// Place mines
	printf("Placing Mines\n");
  gamestate = PlaceMines();
  gamestate.minesLeft = NUM_MINES;
	printf("Mines placed\n");

	while (playing) {
		res = 1;

		// Format and print gamestate
		FormatGameState(gamestate, gameString);

		for(int i = 0; i < MAXGAMESIZE+1; i++){
			printf("%c,", gameString[i]);
		}
		printf("\n");

		// Send gamestate
		printf("Sending gamestate\n");
		shortRetval = SendData(socket_id, gameString, sizeof gameString);

		// Wait for chosen option from client
    shortRetval = ReceiveData(socket_id, chosenOption, 8);

    printf("Received Data: %s", chosenOption);
    printf("\n");
		printf("%c,%c\n", chosenOption[1], chosenOption[2]);

    if (chosenOption[0] == 'r'){
      // Flip Tile
      printf("User chose to Flip Tile\n");
			FlipTile(&gamestate, chosenOption[1] - 49, chosenOption[2] - 49);
    } else if (chosenOption[0] == 'p'){
      // Place Flag
      printf("User chose to Place Flag\n");
			FlagTile(&gamestate, chosenOption[1] - 49, chosenOption[2] - 49, socket_id);
    } else if (chosenOption[0] == 'q'){
      // User chose to quit
      playing = 0;
      printf("User chose to quit\n");
			break;
    } else {
      printf("Error with string\n");
    }
	}
}

// Generic Receive Data function
int ReceiveData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = recv(serverSocket, message, messageSize, 0)) <= 0 ) {
    perror("recv");
    exit(1);
  }
  // message[1]='\0';
  return shortRetval;
}

// Generic Send Data function
int SendData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = send(serverSocket, message, messageSize, 0)) == -1) {
    perror("send");
		exit(1);
  }
  return shortRetval;
}

// Returns whether the location given is a mine
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
			gamestate.tiles[x_tile][y_tile].is_mine = false;
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
						if(gamestate.tiles[x+a][y+b].is_mine == false){
							gamestate.tiles[x+a][y+b].adjacent_mines++;
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
			loc = x * NUM_TILES_X + y;
      if(gamestate.tiles[x][y].revealed == true){
				if (gamestate.tiles[x][y].is_mine == true){
					gameString[loc] = '*';
				} else {
					sprintf(&gameString[loc], "%d", gamestate.tiles[x][y].adjacent_mines);
				}
			} else{
        gameString[loc] = ' ';
			}
		}
	}

	if (gamestate.minesLeft == 10) {
		gameString[82] = '1';
		gameString[83] = '0';
	} else {
		gameString[82] = ' ';
		gameString[83] = gamestate.minesLeft+'0';
	}
}

// Function sorts the leaderboard in descending order by seconds, total won, username (alpha)
void SortLeaderboard(struct LeaderboardEntry *leaderboard) {
	int x = 0, y;
	struct LeaderboardEntry temp;

	for (y = 0; y <= x; y++) {
		for(x = 0; x < sizeof(leaderboard); x++) {
			if (leaderboard[x].time < leaderboard[x + 1].time) {
				// Time is less, reorder
				temp = leaderboard[x];
				leaderboard[x] = leaderboard[x + 1];
				leaderboard[x + 1] = temp;
			} else if (leaderboard[x].time == leaderboard[x + 1].time) {
				// Time is equal, check total won
				if (leaderboard[x].won > leaderboard[x + 1].won) {
					// Total won is less, reorder
					temp = leaderboard[x];
					leaderboard[x] = leaderboard[x + 1];
					leaderboard[x + 1] = temp;
				} else if (leaderboard[x].won == leaderboard[x + 1].won) {
					// Total win is equal, check alphabetical by username
					int username_length = 0;

					// Determine the bigger username
					if (strlen(leaderboard[x].username) >= strlen(leaderboard[x + 1].username)) {
						username_length = strlen(leaderboard[x].username);
					} else {
						username_length = strlen(leaderboard[x + 1].username);
					}

					// Iterate through each char and compare
					for (int i = 0; i < username_length; i++) {
						if (leaderboard[x].username[i] < leaderboard[x + 1].username[i]) {
							// Reorder to alphabetical order
							temp = leaderboard[x];
							leaderboard[x] = leaderboard[x + 1];
							leaderboard[x + 1] = temp;
						}
					}
				}
			}
		}
	}
}

// Function formats the leaderboard and sends it to the client
void SendLeaderboard(int socket, struct LeaderboardEntry *leaderboard) {
	int time_count, won, played;

	SortLeaderboard(leaderboard);
	fprintf(stderr, "Leaderboard Sorted\n");

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

// Flip the tile requested by the client
void FlipTile(struct GameState *gameState, int loc_x, int loc_y){
	int x_tile, y_tile;
	x_tile = loc_x;
	y_tile = loc_y;
	// Check to see if tile is a mine
	if( (*gameState).tiles[x_tile][y_tile].is_mine == true){
		// Game Over
		printf("Game Over: Mine at %d/%d\n", x_tile, y_tile);
		(*gameState).GameOver = true;
		for(int x = 0; x < NUM_TILES_X; x++){
			for (int y = 0; y < NUM_TILES_Y; y++){
				if ((*gameState).tiles[x][y].is_mine == true){
					(*gameState).tiles[x][y].revealed = true;
				} else {
					(*gameState).tiles[x][y].revealed = false;
				}
			}
		}
	} else {
		// If tile is not a mine - flip the tile, to reveal number below
		(*gameState).tiles[x_tile][y_tile].revealed = true;
		printf("Flipped Tile %d/%d\n", x_tile, y_tile);
	}
	if ((*gameState).tiles[x_tile][y_tile].adjacent_mines == 0){
		for(int a = -1; a < 2; a++){
			for (int b = -1; b < 2; b++){
				if((a + x_tile > -1) && (a + x_tile < NUM_TILES_X)){
					if((b + y_tile > -1) && (b + y_tile < NUM_TILES_Y)){
						if((*gameState).tiles[x_tile+a][y_tile+b].adjacent_mines == 0){
							if ((*gameState).tiles[x_tile+a][y_tile+b].revealed == false){
								FlipTile(gameState, x_tile+a, y_tile+b);
							}
						}
					}
				}
			}
		}
	}
	// If tile is a 0, flip the tiles around it. Repeat (Most likely call FlipTile for the tiles around)
}

// Flag a tile, if a mine = success, if not = inform client / display message
void FlagTile(struct GameState *gameState, int loc_x, int loc_y, int socket_id){
	char flagMessage[2];

	// Check to see if tile is mine
	if ((*gameState).tiles[loc_x][loc_y].is_mine == true) {
		// Flag placed successfully
		printf("Flag Placed! Mine at %d/%d\n", loc_x, loc_y);

		flagMessage[0] = '1';

		(*gameState).tiles[loc_x][loc_y].revealed = true;
		(*gameState).minesLeft = (*gameState).minesLeft - 1;

		printf("Mines left: %d\n", (*gameState).minesLeft);

		// Successfully placed a flag - Check for win state
		if ((*gameState).minesLeft == 0) {
			// WE WIN! Stop timer, send message and time
			end_time = time(NULL);
			int seconds_taken = difftime(start_time, end_time);
			// send win notification and time to client
			flagMessage[1] = '1';
		} else {
			// More mines are remaining
			flagMessage[1] = '0';
		}
	} else {
		// No mine at loc, flag not placed
		// send message to user and break;
		printf("No mine at: %d, %d\n", loc_x, loc_y);

		flagMessage[0] = '0';
	}

	// Send status to user
	SendData(socket_id, flagMessage, sizeof flagMessage);
}

void GameOverMsg(int time, int won){
	if (won){
		// Send Congrats

	} else {
		// Send Gameover
	}

}
