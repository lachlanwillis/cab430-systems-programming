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
  int portNum, clientConnect, connectionStatus, numbytes;
  char buf[MAXDATASIZE];
  char * inetAddr;
  char testMessage[256];
  struct sockaddr_in serverAddress;

  if (argc == 3) {
    // User provided IP and Port
    portNum = (unsigned short) atoi(argv[2]);
    inetAddr = argv[1];

    printf("IP and Port Provided - using %s:%d\n", inetAddr, portNum);
  } else if (argc == 1) {
    // User did not provide any information - use defaults
    portNum = 12345;
    inetAddr = "192.168.1.120";

    printf("No IP and Port provided - using default 192.168.1.120:12345\n");
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
  
  if ((numbytes = recv(clientConnect, buf, MAXDATASIZE, 0)) == -1) {
    perror("recv");
		exit(1);
  }

  buf[numbytes] = '\0';
  
  // print the data we got
  fprintf(stderr, "%s\n", buf);

  StartMinesweeper();

  while(1) {};

  // Close connection
  close(clientConnect);

  return 0;
}
