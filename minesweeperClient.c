#include "minesweeperClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXDATASIZE 256
#define LEADERBOARD_SIZE 10
# define MAXGAMESIZE 84

int receivedData, sentData;
char username[MAXDATASIZE], password[MAXDATASIZE];

struct LeaderboardEntry {
	char username[MAXDATASIZE];
	int time;
	int won;
	int played;
};

struct LeaderboardEntry leaderboard[LEADERBOARD_SIZE];

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
  char selectionOption[256];

  while (selection == 0) {
    fprintf(stderr, "Please enter a selection:\n");
    fprintf(stderr, "<1> Play Minesweeper\n");
    fprintf(stderr, "<2> Show Leaderboard\n");
    fprintf(stderr, "<3> Quit\n");
    fprintf(stderr, "\nSelection option (1-3):");

    scanf("%s", selectionOption);

    int shortRetval = SendData(serverSocket, selectionOption, strlen(selectionOption));

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

int ReceiveLeaderboard(int socket, int size) {
  int number_of_bytes;
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

// Displays the Leaderboard - Requires Communication to Server
void ShowLeaderboard(int serverSocket){
  fprintf(stderr, "=================================================================\n");
  fprintf(stderr, "Current Minesweeper Leaderboard\n");
  fprintf(stderr, "=================================================================\n");

  int shortRetval = -1;

  // Get updated data from server
  shortRetval = ReceiveLeaderboard(serverSocket, LEADERBOARD_SIZE);

  // Show the leaderboard
  for (int i = 0; i < LEADERBOARD_SIZE; i++) {
    fprintf(stderr, "%s - ", leaderboard[i].username);
    fprintf(stderr, "%d - ", leaderboard[i].time);
    fprintf(stderr, "%d - ", leaderboard[i].won);
    fprintf(stderr, "%d\n", leaderboard[i].played);
  }
}

// Start Playing the game Minesweeper
void PlayMinesweeper(int serverSocket){
  int playingGame = 1, enteringOption = 1;
  while(playingGame){
    // Get Data from Server here.
		char gamestate[MAXGAMESIZE];
		strcpy(gamestate, ReceiveGameState(serverSocket));
		printf("Drawing Game\n");
    // Draw Tiles
    DrawGame(gamestate);

    while(enteringOption){
      char selectionOption[256];
      scanf("%s", selectionOption);

      if (strcmp("r", selectionOption) == 0){
        // User chose to reveal a tile
        printf("Enter tile coordinates: ");
        char* chosenTile;
        scanf("%s", chosenTile);

      } else if (strcmp("p", selectionOption) == 0){
        // User chose to place a flag
        printf("Enter tile coordinates: ");
        char* chosenTile;
        scanf("%s", chosenTile);

      } else if (strcmp("q", selectionOption) == 0){
        // User chose to quit
        playingGame = 0;
        system("clear");
        return;

      } else {
      printf("Did not enter Options R, P or Q.\n Please try again\n");
      }
    }
  }
}


void DrawGame(char gameState[MAXGAMESIZE]){
	printf("Drawing Game\n");
  char minesLeft[2];
	if(strcmp(&gameState[82], "0")){
		strcpy(minesLeft, &gameState[MAXGAMESIZE]);
	}else {
		strcpy(minesLeft, "10");
	}
  int x, y;
  printf("Remaining mines: %s\n\n", minesLeft);
  printf("      1 2 3 4 5 6 7 8 9\n");
  printf("  ---------------------\n");

  for(x = 0; x < 9; x++){
    char row;
    int asciCon = x+65;
    row = (char) asciCon;
    printf("  %c | ", row);
    for(y = 0; y < 9; y++){

      printf(" %s", &gameState[x*9+y]);
    }
    printf("\n");
  }

  printf("\n\nChoose an option:\n");
  printf("<R> Reveal tile\n");
  printf("<P> Place flag\n");
  printf("<Q> Quit game\n\n");
  printf("Option (R, P, Q):");

}

char *ReceiveGameState(int serverSocket){
	char gameString[MAXGAMESIZE];
	char *return_str = gameString;
	printf("Receiving Data Minesweeper\n");
	int shortRetval = ReceiveData(serverSocket, gameString, MAXGAMESIZE);
	printf("Received Minesweeper Data\n");
	return return_str;
}
