#include "minesweeperClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAXDATASIZE 256

int receivedData, sentData;
char username[MAXDATASIZE], password[MAXDATASIZE];



// State functions

int DisplayMenu();
void PlayMinesweeper(int);
void ShowLeaderboard(int);
void DrawGame();



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
    int chosenOption = DisplayMenu();
    int commOption = -1;
    if (chosenOption == 1){
      PlayMinesweeper(serverSocket);
    }else if(chosenOption == 2){
      ShowLeaderboard(serverSocket);
    }else if(chosenOption == 3){
      strcpy(message, "3");
      commOption = SendData(serverSocket, message, MAXDATASIZE);
      system("clear");
      exit(EXIT_SUCCESS);
    }
  }

}


int DisplayMenu(){
  int chosen = 0;
  while(chosen != 1){
    fprintf(stderr, "Please enter a selection:\n");
    fprintf(stderr, "<1> Play Minesweeper\n");
    fprintf(stderr, "<2> Show Leaderboard\n");
    fprintf(stderr, "<3> Quit\n");
    fprintf(stderr, "\nSelection option (1-3):");

    char* selectionOption;
    scanf("%s", selectionOption);
    if (strcmp("1", selectionOption) == 0){
      // Start Minesweeper
      return(1);
      chosen = 1;
    } else if (strcmp("2", selectionOption) == 0){
      // Show Leaderboard
      return(2);
      chosen = 1;
    } else if (strcmp("3", selectionOption) == 0){
      // Quit
      return(3);
      chosen = 1;
    } else {
      // Incorrect Input
      fprintf(stderr, "Did not enter 1-3, please try again.\n\n");
    }
  }

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


// Displays the Leaderboard - Requires Communication to Server
void ShowLeaderboard(int serverSocket){

}


// Start Playing the game Minesweeper
void PlayMinesweeper(int serverSocket){
  int playingGame = 1, enteringOption = 1;
  while(playingGame){
    // Get Data from Server here.


    // Draw Tiles
    DrawGame();

    while(enteringOption){
      char* selectionOption;
      scanf("%s", selectionOption);

      if (strcmp("R", selectionOption) == 0){
        // User chose to reveal a tile
        printf("Enter tile coordinates: ");
        char* chosenTile;
        scanf("%s", chosenTile);

      } else if (strcmp("P", selectionOption) == 0){
        // User chose to place a flag
        printf("Enter tile coordinates: ");
        char* chosenTile;
        scanf("%s", chosenTile);

      } else if (strcmp("Q", selectionOption) == 0){
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


void DrawGame(){
  //char gameState[][], int minesLeft // Move this into the arguments passed once we ****** RECEIVE DATA FROM SERVER*****
  int minesLeft = 10; // Once we have set up receiving data from Server ******REMOVE THIS******
  int x, y;
  printf("Remaining mines: %d\n\n", minesLeft);
  printf("      1 2 3 4 5 6 7 8 9\n");
  printf("  ---------------------\n");

  for(x = 0; x < 9; x++){
    char row;
    int asciCon = x+65;
    row = (char) asciCon;
    printf("  %c | ", row);
    for(y = 0; y < 9; y++){
      printf(" %s", " ");
    }
    printf("\n");
  }

  printf("\n\nChoose an option:\n");
  printf("<R> Reveal tile\n");
  printf("<P> Place flag\n");
  printf("<Q> Quit game\n\n");
  printf("Option (R, P, Q):");

}
