#include "minesweeperClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <curses.h>


#define MAXDATASIZE 256
#define LEADERBOARD_SIZE 10
# define MAXGAMESIZE 83

#define NUM_TILES_X 9
#define NUM_TILES_Y 9

int receivedData, sentData;
char username[MAXDATASIZE], password[MAXDATASIZE];

struct LeaderboardEntry {
	char username[MAXDATASIZE];
	int time;
	int won;
	int played;
};

struct LeaderboardEntry leaderboard[LEADERBOARD_SIZE];

char gameString[MAXGAMESIZE];

// create functions here that are defined in the header
void StartMinesweeper(int serverSocket) {
  int shortRetval = -1, loginStatus = -1;
  char message[MAXDATASIZE];
  char loginMessage[strlen(message)];

  fprintf(stderr, "=================================================================\n");
  fprintf(stderr, "Welcome to the online Minesweeper gaming system\n");
  fprintf(stderr, "=================================================================\n\n");

  // Get username
  fprintf(stderr, "You are required to log on with your registered name and password:\n\n");
  while(strcmp(loginMessage, "1")!=0){


    fprintf(stderr, "Username: "); //Add read input
    scanf("%s", username);

    shortRetval = SendData(serverSocket, username, strlen(username));

    // Get password
    fprintf(stderr, "Password: "); //Add read input
    scanf("%s", password);

    shortRetval = SendData(serverSocket, password, strlen(password));

    // wait for server response
    loginStatus = ReceiveData(serverSocket, message, MAXDATASIZE);
    strcpy(loginMessage, message);
    if(strcmp(loginMessage, "1")==0){
      // Logged in Successfully
      printf("%s\n", "Successfully Logged in");
    } else{
      fprintf(stderr, "Incorrect Username or Password. Please try again...\n");
    }
  }
  // At this point the user is logged in and can proceed to menu
  system("clear");
  while(1){
    int chosenOption = DisplayMenu(serverSocket);
    int commOption = -1;

    if (chosenOption == 1){
      PlayMinesweeper(serverSocket);
    } else if(chosenOption == 2){
      system("clear");
      ShowLeaderboard(serverSocket);
    } else if(chosenOption == 3){
      strcpy(message, "3");
      commOption = SendData(serverSocket, message, MAXDATASIZE);
      system("clear");
      exit(EXIT_SUCCESS);
    }
  }
}

int DisplayMenu(int serverSocket){
  int selection = 0;
  char selectionOption[MAXDATASIZE];

  while (selection == 0) {
    fprintf(stderr, "Please enter a selection:\n");
    fprintf(stderr, "<1> Play Minesweeper\n");
    fprintf(stderr, "<2> Show Leaderboard\n");
    fprintf(stderr, "<3> Quit\n");
    fprintf(stderr, "\nSelection option (1-3):");

    scanf("%s", selectionOption);

    SendData(serverSocket, selectionOption, strlen(selectionOption));

    if (strcmp("1", selectionOption) == 0){
      // Start Minesweeper
      selection = 1;
    } else if (strcmp("2", selectionOption) == 0){
      // Show Leaderboard
      selection = 2;
    } else if (strcmp("3", selectionOption) == 0){
      // Quit
      selection = 3;
    } else {
      // Incorrect Input
      printf("Printed: %s\n", selectionOption);
      fprintf(stderr, "Did not enter 1-3, please try again.\n\n");
    }
  }
  return selection;
}


// General purpose function for receiving data from server
int ReceiveData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = recv(serverSocket, message, messageSize, 0)) == -1) {
    perror("recv");
		exit(1);
  }
  message[shortRetval]='\0';
  return shortRetval;
}

// General purpose function for sending data to server
int SendData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = send(serverSocket, message, messageSize, 0)) == -1) {
    perror("send");
		exit(1);
  }
  return shortRetval;
}

int ReceiveLeaderboard(int socket, int size) {
  int number_of_bytes = 0;
  int time_count = 0, played = 0, won = 0;
  char recv_username[MAXDATASIZE];

	for (int i = 0; i < size; i++) {
    int shortRetval = ReceiveData(socket, recv_username, MAXDATASIZE);
    shortRetval = read(socket, &time_count, sizeof(time_count));
    shortRetval = read(socket, &won, sizeof(won));
    shortRetval = read(socket, &played, sizeof(played));

    strcpy(leaderboard[i].username, recv_username);
    leaderboard[i].time = ntohl(time_count);
    leaderboard[i].won = ntohl(won);
    leaderboard[i].played = ntohl(played);
	}

	return number_of_bytes;
}

// Moves cursor to x y pos on terminal
void gotoxy(int x, int y) {
  printf("\033[%d;%df", y, x);
  fflush(stdout);
}

// Displays the Leaderboard - Requires Communication to Server
void ShowLeaderboard(int serverSocket){
  fprintf(stderr, "===========================================================================\n");
  fprintf(stderr, "Current Minesweeper Leaderboard\n");
  fprintf(stderr, "===========================================================================\n");
  fprintf(stderr, "USERNAME");
  gotoxy(20, 4);
  fprintf(stderr, "BEST TIME");
  gotoxy(40, 4);
  fprintf(stderr, "GAMES WON / TOTAL PLAYED\n");
  fprintf(stderr, "===========================================================================\n");

  int shortRetval = -1;

  // Get updated data from server
  shortRetval = ReceiveLeaderboard(serverSocket, LEADERBOARD_SIZE);

  // Show the leaderboard
  for (int i = 0; i < LEADERBOARD_SIZE; i++) {
    if (leaderboard[i].username[0] != '\0') {
      fprintf(stderr, "%s", leaderboard[i].username);
      gotoxy(20, 6 + i);
      fprintf(stderr, "%d seconds", leaderboard[i].time);
      gotoxy(40, 6 + i);
      fprintf(stderr, "%d games won, ", leaderboard[i].won);
      fprintf(stderr, "%d games played\n", leaderboard[i].played);
    }
  }
  fprintf(stderr, "\n");
}

// Start Playing the game Minesweeper
void PlayMinesweeper(int serverSocket){
  int playingGame = 1;
  do {
		int enteringOption = 1;
		// Send message to sever requesting gameState
		printf("Requesting Data from server\n");
		int shortRetval = -1;
		shortRetval = SendData(serverSocket, "1", 1);
		if (shortRetval < 0){
			printf("Error communicating with server\n");
		}
    // Get Data from Server here.
		ReceiveGameState(serverSocket, gameString);
		printf("Drawing Game: %s\n", gameString);
    // Draw Tiles
    DrawGame(gameString);

    while(enteringOption){
      char selectionOption[256];
      scanf("%s", selectionOption);

      if (strcmp("r", selectionOption) == 0){
        // User chose to reveal a tile
				int coords = GetTileCoordinates();
        SendGameChoice(serverSocket, "r", coords);
				printf("Sent data to server\n");
				enteringOption = 0;

      } else if (strcmp("p", selectionOption) == 0){
        // User chose to place a flag
        int coords = GetTileCoordinates();
        SendGameChoice(serverSocket, "p", coords);
				enteringOption = 0;

      } else if (strcmp("q", selectionOption) == 0){
        // User chose to quit
        playingGame = 0;
        SendGameChoice(serverSocket, "q", 0);
        system("clear");
        return;
      } else {
        printf("Did not enter Options R, P or Q.\n Please try again\n");
      }
    }
  } while(playingGame);
}

// Prints commands to user and deciphers inputs.
int GetTileCoordinates(){
	while(1){
		printf("Enter tile coordinates: ");
		char chosenTile[256];
		scanf("%s", chosenTile);
		// Convert letter to number
		char letterResult = toupper(chosenTile[0]) - 'A' + 1;
		int letterCoord = letterResult;
		int numCoord = chosenTile[1] - 48;

		// Check to make sure letter and Number is in range
		if(letterCoord < 1 || letterCoord > NUM_TILES_Y){
			printf("Please enter a letter between A and I\n");
		} else {
			if (numCoord < 1 || numCoord > NUM_TILES_X){
				printf("Please enter a number between 1 and 9\n");
			} else {
				return(letterCoord*10+numCoord);
			}

		}

	}
}


// Draws the gamestate to the user with the provided char
void DrawGame(char* gameState){
  char* minesLeft;
  int x, y, asciCon;
  char row;

	printf("Drawing Game\n");
  system("clear");

	if (gameState[82] == '0'){
		minesLeft = &gameState[MAXDATASIZE];
	} else {
		minesLeft = "10";
	}

  printf("Remaining mines: %s\n\n", minesLeft);
  printf("      1 2 3 4 5 6 7 8 9\n");
  printf("  ---------------------\n");

  for(x = 0; x < 9; x++){
		// Convert number to ASCI to display GRID
    asciCon = x + 65;
    row = (char) asciCon;
    printf("  %c |", row);
    for(y = 0; y < 9; y++){
			// Print the tiles
      printf(" %c", gameState[x*9+y]);
    }
    printf("\n");
  }

  printf("\n\nChoose an option:\n");
  printf("<R> Reveal tile\n");
  printf("<P> Place flag\n");
  printf("<Q> Quit game\n\n");
  printf("Option (R, P, Q):");

}

// Receives string with gamestate from server
void ReceiveGameState(int serverSocket, char* gameString){
	int recData = -1;
	while(recData < 0){
		printf("Receiving Data Minesweeper\n");
		recData = ReceiveData(serverSocket, gameString, MAXGAMESIZE+1);
		printf("Received Minesweeper Data\n");
	}

}

// Sends chosen tile and option to server.
void SendGameChoice(int serverSocket, char* chosenOption, int tileLoc){
  int res;
	char tileRes[64], messageToSend[MAXDATASIZE];
  char *msg = messageToSend;

  printf("SENDING GAME CHOICE");
	sprintf(tileRes, "%d", tileLoc);
  strcpy(&messageToSend[0], chosenOption);
  strcpy(&messageToSend[1], tileRes);

  printf("%s\n", msg);
  res = SendData(serverSocket, msg, MAXDATASIZE);
}
