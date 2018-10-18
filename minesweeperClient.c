#include "minesweeperClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAXDATASIZE 256

int receivedData, sentData;
char username[MAXDATASIZE], password[MAXDATASIZE];

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
    printf("%s\n", loginMessage);
    if(strcmp(loginMessage, "1")==0){
      // Logged in Successfully
      printf("%s\n", "Successfully Logged in");
    } else{
      fprintf(stderr, "Incorrect Username or Password. Please try again...\n");
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
