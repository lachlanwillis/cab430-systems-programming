#include "minesweeperClient.h"
#include <stdio.h>

#define MAXDATASIZE 256

char buf[MAXDATASIZE];

// create functions here that are defined in the header
void StartMinesweeper() {
  fprintf(stderr, "=================================================================\n");
  fprintf(stderr, "Welcome to the online Minesweeper gaming system\n");
  fprintf(stderr, "=================================================================\n\n");

  fprintf(stderr, "You are required to log on with your registered name and password:\n\n");
  fprintf(stderr, "Username: "); //Add read input
  scanf("%s", buf);
  fprintf(stderr, "You entered: %s", buf);
}