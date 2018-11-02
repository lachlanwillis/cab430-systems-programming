#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include <string.h>
#include <inttypes.h>
#include <malloc.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include <unistd.h>

#include "minesweeperServer.h"


#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

#define TOTAL_CONNECTIONS 10
#define MAXDATASIZE 256
#define RANDOM_NUM_SEED 42
#define THREAD_POOL_SIZE 10

// Setup leaderboard array
struct LeaderboardEntry leaderboard[TOTAL_CONNECTIONS];

struct Request {
	int number;
	int sockfd;
	struct Request *next;
};

void ClientConnectionsHandler(struct Request*, int);
void HandleExitSignal();
void ClientCommunicationHandler(int, char *[256]);
int NumAuths(char *);
void MinesweeperMenu(int);
void SetupThreadPool();
void SetupMutex();
void* HandleConnections(void *);
void ClientRequestAdd(int socket_id, int num_request, pthread_mutex_t *pthread_mutex, pthread_cond_t *pthread_cond_variable);

// Setup server, client socket variables
int serverListen, clientConnect, portNum;
struct sockaddr_in serv_addr, client;
socklen_t sin_size;
int clientTotalRequests = 0, totalRequests = 0;

struct Request *requests = NULL;
struct Request *last_request = NULL;

// Setup pthread variables
pthread_t client_thread[THREAD_POOL_SIZE];
int attr[THREAD_POOL_SIZE];

// Setup mutex variables
pthread_mutex_t request_mutex;
pthread_mutexattr_t recursiveOptions;

pthread_cond_t request_cond = PTHREAD_COND_INITIALIZER;




int main(int argc, char* argv[]) {
	// Seed the random number
	srand(RANDOM_NUM_SEED);

	// Setup Handle Exit Signal
	//signal(SIGINT, HandleExitSignal);

	// Handle Port Connection
	if (argc < 2) {
		fprintf(stderr, "%s\n", "No Port Provided - using default 12345");
		portNum = 12345;
	} else {
		portNum = atoi(argv[1]);
		printf("Port Provided - using %d\n", portNum);
	}


	SetupMutex();
	SetupThreadPool();

	// Generate server socket
	if ((serverListen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	printf("Generated server socket\n");

	// Setup Server Address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNum);

	// Bind config to server
	if ((bind(serverListen, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) == -1) {
		perror("bind");
		exit(1);
	}
	printf("Bound configuration to server\n");

	// Start listening
	if ((listen(serverListen, TOTAL_CONNECTIONS)) == -1) {
		perror("listen");
		exit(1);
	}
	printf("Server started listnening...\n");

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if((clientConnect = accept(serverListen, (struct sockaddr *) &client, &sin_size)) == -1) {
			perror("accept");
		} else {
			printf("Server: received connection from %s\n", inet_ntoa(client.sin_addr));
			ClientRequestAdd(clientConnect, totalRequests++, &request_mutex, &request_cond);
		}


		// Create a thread to accept client
		// TODO: Threading does not work with multiple concurrent users.

		// pthread_attr_init(&attr);
		// pthread_create(&client_thread, &attr, ClientConnectionsHandler, (void *) &clientConnect);

		// pthread_join(client_thread, NULL);
	}

	close(clientConnect);
}

// Gets a request from the Queue
struct Request *GetRequests(pthread_mutex_t *pthread_mutex){
	struct Request *request;
	// puts("Getting Request\n");
	pthread_mutex_lock(pthread_mutex);
	// puts("Getting request\n");
	if (clientTotalRequests > 0){
		puts("Finding request\n");
		request = requests;
		requests = requests->next;
		if (requests == NULL){
			last_request = NULL;

		}
		clientTotalRequests--;

	} else {
		// No clients in the Queue
		puts("No client in queue\n");
		request = NULL;
	}

	// unlock the Mutex
	puts("Got Request\n");
	pthread_mutex_unlock(pthread_mutex);
	return request;


}


// Setup the thread pool
void SetupThreadPool(){
	puts("Creating Threads\n");
	for (int i = 0; i < THREAD_POOL_SIZE; i++){
		attr[i] = i;
		pthread_create(&client_thread[i], NULL, HandleConnections, (void *) &attr[i]);
	}
}


void SetupMutex() {
	// Setup the Mutexes
	puts("Defining Mutexs\n");

	pthread_mutexattr_init(&recursiveOptions);
	pthread_mutexattr_settype(&recursiveOptions, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&request_mutex, &recursiveOptions);


}


void *HandleConnections(void *args){
	struct Request *request;
	int thread_id = *((int*) args);

	// lock the request mutex
	pthread_mutex_lock(&request_mutex);
	printf("Thread%d %lu is waiting for a connection\n", thread_id, client_thread[thread_id]);
	while(1){

		if (clientTotalRequests > 0) {

			request = GetRequests(&request_mutex);
			// Check if not null
			if (request){
				puts("Got Request");
				pthread_mutex_unlock(&request_mutex);
				// Handle the request here
				ClientConnectionsHandler(request, thread_id);
				free(request);
				// Lock the mutex again
				pthread_mutex_lock(&request_mutex);

			}
		} else {
			pthread_cond_wait(&request_cond, &request_mutex);
		}

	}


}


// Function for adding client requests
void ClientRequestAdd(int socket_id, int num_request, pthread_mutex_t *pthread_mutex, pthread_cond_t *pthread_cond_variable){
	// Create a request structure for clients connection
	puts("Starting Adding Request\n");
	struct Request *request;

	request = malloc(sizeof(struct Request));
	// Set values of request struct

	request->number = num_request;
	request->sockfd = socket_id;
	request->next = NULL;

	// Set the next value of the request
	pthread_mutex_lock(pthread_mutex);
	puts("checking connection\n");
	if (clientTotalRequests == 0){
		requests = request;
		last_request = request;

	} else {
		last_request->next = request;
		last_request = request;
	}
	puts("connection checked\n");
	clientTotalRequests++;

	// Unlock mutex and send signal
	pthread_mutex_unlock(pthread_mutex);
	pthread_cond_signal(pthread_cond_variable);

}




// TODO: DOESNT WORK AS EXPECTED
void HandleExitSignal() {
	// Close socket connection
	printf("\n\nClosing server and client sockets\n");
	close (clientConnect);
	close (serverListen);

	// Kill threads and exit program
	printf("Exiting program...\n");

	// Exit program
	exit(1);
}

// Handle client connections
void ClientConnectionsHandler(struct Request *request, int socket_id) {
	char message[MAXDATASIZE], loginMessage[MAXDATASIZE];
	int read_size;
	//int socket_id = *((int *)args);
	char authFile[50] = "Authentication.txt";
	char loginDetails[13][2][256];

	FILE *fd;
	fd = fopen(authFile, "r");
	int x, y;
	for (x = 0; x < 12; x++){
		for(y = 0; y < 2; y++){
			fscanf(fd, "%s", loginDetails[x][y]);
		}
	}
	fclose(fd);

	while(strcmp(loginMessage, "1")!=0){
		// Receive username
		int user = -1;
		strcpy(loginMessage, "0");

		memset(message,0,sizeof(message));

		read_size = ReceiveData(request->sockfd, message, MAXDATASIZE);

		fprintf(stderr, "Received username: %s\n", message);
		for(int i = 1; i < 12; i++){
			if(strcmp(message, loginDetails[i][0]) == 0){
				user = i;
			}
		}

		memset(message,0,sizeof(message));

		// Receive password
		read_size = ReceiveData(request->sockfd, message, MAXDATASIZE);

		fprintf(stderr, "Received password: %s\n", message);
		int resMes = -1;
		if((user > 0) && (strcmp(message, loginDetails[user][1]) == 0)){
			// Continue
			printf("Correct Username and Password\n");
			strcpy(message, "1");
			strcpy(loginMessage, "1");

		}else{
			printf("Wrong Username or Password\n");
			strcpy(message, "");
			strcpy(loginMessage, "0");
		}
		resMes = SendData(request->sockfd, message, MAXDATASIZE);

	}

	printf("Client logged in successfully\n");
	// Await information on what the client wishes to do

	int clientFinished = 0;
	while(clientFinished != 1){
		ReceiveData(request->sockfd, message, MAXDATASIZE);

		if (strcmp("1", message) == 0){
	    // Start Minesweeper
			MinesweeperMenu(request->sockfd);
			printf("Ended, waiting for next command\n");
	  } else if (strcmp("2", message) == 0){
	    // Show Leaderboard
			printf("Sending leaderboard\n");
			SendLeaderboard(request->sockfd);
			printf("Sent leaderboard\n");
	  } else if (strcmp("3", message) == 0){
	    // Quit
			printf("Client Disconnecting\n");
	    close(request->sockfd);
			pthread_join(client_thread, NULL);
			clientFinished = 1;
	  }
	}

}
