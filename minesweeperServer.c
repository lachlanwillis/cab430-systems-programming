#include "minesweeperServer.h"
#include <stdio.h>

// create functions here that are defined in the header
int ReceiveData(int serverSocket, char* message, short messageSize) {
  int shortRetval = -1;
  int *results = malloc(sizeof(int)*messageSize);

  if ((shortRetval = recv(serverSocket, &message, messageSize, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  results = ntohs(message);

  printf("%d", results);

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