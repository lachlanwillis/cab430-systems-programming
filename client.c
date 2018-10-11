#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>


int main(int argc, char* argv[]){
  // Setup port and connection address
  int portNum;
  char * inetAddr;

  if (argc == 3) {
    // User provided IP and Port
    portNum = (unsigned short) atoi(argv[2]);
    inetAddr = argv[1];

    printf("IP and Port Provided - using %s:%d", inetAddr, portNum);
  } else if (argc == 1) {
    // User did not provide any information - use defaults
    portNum = 12345;
    inetAddr = "127.0.0.1";

    fprintf(stderr, "%s\n", "No IP and Port provided - using default localhost:12345");
  } else {
    // Error in provided information
    printf("Usage: %s hostname port -OR- %s", argv[0], argv[0]);
    exit(1);
  }

  // Setup the client connection
  int clientConnect = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in  serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port= htons(portNum);
  serverAddress.sin_addr.s_addr = inet_addr(inetAddr);

  // Attempt to connect to the server
  int connectionStatus = connect(clientConnect, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
  if (connectionStatus == -1){
    // Problem connecting to server
    fprintf(stderr, "%s\n", "There was a problem connecting to socket\n");
  }

  char testMessage[256];
  recv(clientConnect, &testMessage, sizeof(testMessage), 0);
  // print the data we got
  fprintf(stderr, "%s\n", testMessage);

  // Close connection
  close(clientConnect);

  return 0;
}
