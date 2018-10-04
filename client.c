#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>


int main(int argc, char* argv[]){
  // Setup port and connection address
  int portNum, conAddr;
  if(argc < 3){
    fprintf(stderr, "%s\n", "ERROR, No String Provided!");
    exit(1);
  } else {
    portNum = atoi(argv[2]);
    conAddr = gethostbyname(argv[1]);
  }


  // Setup the client connection
  int clientConnect = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in  serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port= htons(portNum);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Attempt to connect to the server
  int connectionStatus = connect(clientConnect, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
  if (connectionStatus = -1){
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
