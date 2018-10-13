#include "minesweeperClient.h"
#include <stdio.h>

#define MAXDATASIZE 256

int receivedData, sentData;
char username[MAXDATASIZE], password[MAXDATASIZE];

// create functions here that are defined in the header
void StartMinesweeper(int serverSocket) {
  int shortRetval = -1;

  fprintf(stderr, "=================================================================\n");
  fprintf(stderr, "Welcome to the online Minesweeper gaming system\n");
  fprintf(stderr, "=================================================================\n\n");

  // Get username
  fprintf(stderr, "You are required to log on with your registered name and password:\n\n");
  fprintf(stderr, "Username: "); //Add read input
  scanf("%s", username);
  fprintf(stderr, "\n");

  shortRetval = SendData(serverSocket, username, strlen(username));

  // Get password
  fprintf(stderr, "Password: "); //Add read input
  // scanf("%s", password);
  // fprintf(stderr, "\n\n");

  // Send authentication data to server
  // shortRetval = SendData(serverSocket, password, strlen(password));
}

int ReceiveData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;

  if ((shortRetval = recv(serverSocket, message, messageSize, 0)) == -1) {
    perror("recv");
		exit(1);
  }
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