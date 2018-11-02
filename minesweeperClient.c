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
#define MAXGAMESIZE 83

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
int flippedTile[MAXGAMESIZE];

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

      memset(username,0,sizeof(username));
      memset(password,0,sizeof(password));
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
      break;
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

  if ((shortRetval = recv(serverSocket, message, messageSize, 0)) <= 0) {
    perror("recv");
		exit(1);
  }
  // message[1]='\0';
  return shortRetval;
}

// General purpose function for sending data to server
int SendData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  message[messageSize] = '\0';

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
  // Vars to check if any leaderboard entries
  bool hasEntry = false;

  printf("===========================================================================\n");
  printf("Current Minesweeper Leaderboard\n");
  printf("===========================================================================\n");
  printf("USERNAME");
  gotoxy(20, 4);
  printf("BEST TIME");
  gotoxy(40, 4);
  printf("GAMES WON / TOTAL PLAYED\n");
  printf("===========================================================================\n\n");

  int shortRetval = -1;

  // Get updated data from server
  shortRetval = ReceiveLeaderboard(serverSocket, LEADERBOARD_SIZE);

  // Show the leaderboard
  for (int i = 0; i < LEADERBOARD_SIZE; i++) {
    if (leaderboard[i].username[0] != '\0') {
      hasEntry = true;

      printf("%s", leaderboard[i].username);
      gotoxy(20, 7 + i);
      printf("%d seconds", leaderboard[i].time);
      gotoxy(40, 7 + i);
      printf("%d games won, ", leaderboard[i].won);
      printf("%d games played\n", leaderboard[i].played);
    }
  }

  if (!hasEntry) {
    // No entries, display message
    printf("There is no info currently stored in the leaderboard. Try again later.\n");
  }
  printf("\n===========================================================================\n\n");
}

// Start Playing the game Minesweeper
void PlayMinesweeper(int serverSocket){
  int playingGame = 1, enteringOption = 1, coords;
  char selectionOption[256];
  char flagMessage[MAXDATASIZE], flipMessage[MAXDATASIZE];
  char coordsChar[3];
  bool flagOption = false, flipOption = false;

  do {
		enteringOption = 1;

    // Get Data from Server here.
		ReceiveGameState(serverSocket, gameString);
		printf("Drawing Game: %s\n", gameString);

    system("clear");

    // Draw Tiles
    DrawGame(gameString);

    if (flagOption) {
      if (flagMessage[0] == '1') {
        // Success, mine at flag loc
        printf("\nSuccess! Mine at location: %c, %c", coordsChar[0], coordsChar[1]);
      } else if (flagMessage[0] == '2') {
        // Tile already flipped
        fprintf(stderr, "\nTile already flipped, pick another.");
      } else {
        // Error, no mine at flag loc
        printf("\nUnsuccessful! Mine NOT at location: %c, %c", coordsChar[0], coordsChar[1]);
      }
      flagOption = false;
    }

    if (flipOption) {
      if (flipMessage[0] == '2') {
        // Tile already flipped
        fprintf(stderr, "\nTile already flipped, pick another.");
      } else {
        // Error, no mine at flag loc
        printf("\nTile flipped: %c, %c", coordsChar[0], coordsChar[1]);
      }
      flipOption = false;
    }

    while(enteringOption){
      printf("\n\nChoose an option:\n");
      printf("<R> Reveal tile\n");
      printf("<P> Place flag\n");
      printf("<Q> Quit game\n\n");
      printf("Option (R, P, Q):");

      scanf("%s", selectionOption);

      if (strcmp("R", selectionOption) == 0){
        // User chose to reveal a tile
				coords = GetTileCoordinates();
        sprintf(coordsChar, "%d", coords);

        SendGameChoice(serverSocket, "r", coords);
        ReceiveData(serverSocket, flipMessage, MAXDATASIZE);

        if (flipMessage[0] == '1') {
          // Hit mine, game over :(
          SendData(serverSocket, username, sizeof username);
          ReceiveGameState(serverSocket, gameString);
          system("clear");

          DrawGame(gameString);
          fprintf(stderr, "\nGame Over! You hit a mine.\n\n");
          playingGame = 0;
        } else if (flipMessage[0] == '2') {
          // Tile already flipped
          fprintf(stderr, "\nTile already flipped, pick another.\n");
        }

        flipOption = true;
				enteringOption = 0;
      } else if (strcmp("P", selectionOption) == 0){
        // User chose to place a flag
        coords = GetTileCoordinates();
        sprintf(coordsChar, "%d", coords);

        SendGameChoice(serverSocket, "p", coords);
        ReceiveData(serverSocket, flagMessage, MAXDATASIZE);

        if (flagMessage[1] == '1') {
          // We have won!
          int time_total = 0;

          // Send username and receive total seconds taken
          read(serverSocket, &time_total, sizeof(int)*MAXDATASIZE);
          SendData(serverSocket, username, sizeof username);

          // Update gamestate
          ReceiveGameState(serverSocket, gameString);
          system("clear");
          DrawGame(gameString);

          // Post message
          fprintf(stderr, "\nCongratulations! You have located all the mines.\n");
          fprintf(stderr, "You won in %d seconds!\n\n", time_total);
          playingGame = 0;
        }

        // ReceiveGameState(serverSocket, gameString);
        // system("clear");
        DrawGame(gameString);

        flagOption = true;
				enteringOption = 0;
      } else if (strcmp("Q", selectionOption) == 0){
        // User chose to quit
        playingGame = 0;

        SendGameChoice(serverSocket, "q", 0);
        ReceiveGameState(serverSocket, gameString);

        system("clear");

        fprintf(stderr, "User quit game successfully.\n\n");
        return;
      } else {
        system("clear");

        DrawGame(gameString);
        fprintf(stderr, "\nDid not enter Options R, P or Q.\nPlease try again\n");
      }
    }
  } while(playingGame);
}

// Prints commands to user and deciphers inputs.
int GetTileCoordinates(){
  char chosenTile[256], letterResult;
  int letterCoord, numCoord;

	while(1){
		printf("Enter tile coordinates: ");
		scanf("%s", chosenTile);

		// Convert letter to number
		letterResult = toupper(chosenTile[0]) - 'A' + 1;
		letterCoord = letterResult;
		numCoord = chosenTile[1] - 48;

		// Check to make sure letter and Number is in range
		if (letterCoord < 1 || letterCoord > NUM_TILES_Y) {
			printf("Please enter a letter between A and I\n");
		} else {
			if (numCoord < 1 || numCoord > NUM_TILES_X) {
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
      printf(" %c", gameState[(x)*9 + (y)]);
    }
    printf("\n");
  }
}

// Receives string with gamestate from server
void ReceiveGameState(int serverSocket, char* gameString){
	printf("Receiving Data Minesweeper\n");
	ReceiveData(serverSocket, gameString, MAXGAMESIZE+1);
	printf("Received Minesweeper Data\n");
}

// Sends chosen tile and option to server.
void SendGameChoice(int serverSocket, char* chosenOption, int tileLoc){
  int res;
	char messageToSend[MAXDATASIZE];
  char *msg = messageToSend;

  printf("SENDING GAME CHOICE: ");
	sprintf(messageToSend, "%c%d", *chosenOption, tileLoc);
  printf("%s\n", msg);

  res = SendData(serverSocket, msg, sizeof msg);
}
