#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "minesweeperClient.h"

#define MAXDATASIZE 256

int main(int argc, char* argv[]){
  // Setup port and connection address
  int portNum, clientConnect, connectionStatus, read_size;
  char * inetAddr;
  struct sockaddr_in serverAddress;
  char message[MAXDATASIZE];

  if (argc == 3) {
    // User provided IP and Port
    portNum = (unsigned short) atoi(argv[2]);
    inetAddr = argv[1];

    printf("IP and Port Provided - using %s:%d\n", inetAddr, portNum);
  } else if (argc == 1) {
    // User did not provide any information - use defaults
    portNum = 12345;
    inetAddr = "0.0.0.0";

    printf("No IP and Port provided - using default 0.0.0.0:12345\n");
  } else {
    // Error in provided information
    fprintf(stderr, "Usage: %s hostname port -OR- %s\n", argv[0], argv[0]);
    exit(1);
  }

  // Setup the client connection
  if ((clientConnect = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
		exit(1);
  }
  fprintf(stderr, "Connecting to the server socket\n");

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port= htons(portNum);
  serverAddress.sin_addr.s_addr = inet_addr(inetAddr);

  // Attempt to connect to the server
  if ((connect(clientConnect, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) == -1) {
    perror("connect");
		exit(1);
  }
  fprintf(stderr, "Connected to server: %s\n", inetAddr);

  // Read connection status
  // read_size = ReceiveData(clientConnect, message, MAXDATASIZE);
  // fprintf(stderr, "%s\n\n", message);

  StartMinesweeper(clientConnect);

  // Close connection
  close(clientConnect);

  return 0;
}
