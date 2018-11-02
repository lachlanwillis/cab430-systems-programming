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

void* ClientConnectionsHandler(void *);
void HandleExitSignal();
void ClientCommunicationHandler(int, char *[256]);
int NumAuths(char *);
void MinesweeperMenu(int);
void SetupThreadPool();
void SetupMutex();
void HandleConnections(void *);

// Setup server, client socket variables
int serverListen, clientConnect, portNum;
struct sockaddr_in serv_addr, client;
socklen_t sin_size;
int clientTotalRequests;

struct Request *requests = NULL;
struct Request *last_request = NULL;

// Setup pthread variables
pthread_t client_thread[THREAD_POOL_SIZE];
pthread_attr_t attr[THREAD_POOL_SIZE];

// Setup mutex variables
int leaderboard_rc = 0;
pthread_mutex_t leaderboard_mutex_write, leaderboard_mutex_read, leaderboard_mutex_rc;
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

pthread_cond_init request_cond = PTHREAD_COND_INITIALIZER;




int main(int argc, char* argv[]) {
	// Seed the random number
	srand(RANDOM_NUM_SEED);

	// Setup Handle Exit Signal
	signal(SIGINT, HandleExitSignal);

	// TEST LEADERBOARD USER
	strcpy(leaderboard[0].username, "Test McTest");
	leaderboard[0].played = 10;
	leaderboard[0].time = 123;
	leaderboard[0].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[1].username, "Test AA");
	leaderboard[1].played = 10;
	leaderboard[1].time = 123;
	leaderboard[1].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[2].username, "Test 3");
	leaderboard[2].played = 10;
	leaderboard[2].time = 9999999;
	leaderboard[2].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[3].username, "Test 4");
	leaderboard[3].played = 10;
	leaderboard[3].time = 13;
	leaderboard[3].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[4].username, "Bcde");
	leaderboard[4].played = 10;
	leaderboard[4].time = 9999999;
	leaderboard[4].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[5].username, "Test 5");
	leaderboard[5].played = 1000;
	leaderboard[5].time = 9999999;
	leaderboard[5].won = 999;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[6].username, "Abcd");
	leaderboard[6].played = 10;
	leaderboard[6].time = 9999999;
	leaderboard[6].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[7].username, "Test 6");
	leaderboard[7].played = 10;
	leaderboard[7].time = 13;
	leaderboard[7].won = 7;
	// TEST LEADERBOARD USER
	strcpy(leaderboard[8].username, "Test AAA");
	leaderboard[8].played = 10;
	leaderboard[8].time = 123;
	leaderboard[8].won = 7;


	// Handle Port Connection
	if (argc < 2) {
		fprintf(stderr, "%s\n", "No Port Provided - using default 12345");
		portNum = 12345;
	} else {
		portNum = atoi(argv[1]);
		printf("Port Provided - using %d\n", portNum);
	}

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
			ClientRequestAdd(clientConnect, clientTotalRequests++, &request_mutex, &request_cond);
		}


		// Create a thread to accept client
		// TODO: Threading does not work with multiple concurrent users.

		// pthread_attr_init(&attr);
		// pthread_create(&client_thread, &attr, ClientConnectionsHandler, (void *) &clientConnect);

		// pthread_join(client_thread, NULL);
	}

	close(clientConnect);
}



// Setup the thread pool
void SetupThreadPool(){
	for (int i = 0; i < THREAD_POOL_SIZE; i++){
		attr[i] = i;
		pthread_create(&client_thread[i], NULL, HandleConnections, (void *) &attr[i]);
	}
}


void SetupMutex() {
	// Setup the Mutexes
	leaderboard_rc = 0;
	pthread_mutex_init(&leaderboard_mutex_rc, NULL);
	pthread_mutex_init(&leaderboard_mutex_read, NULL);
	pthread_mutex_init(&leaderboard_mutex_write, NULL);
}


void HandleConnections(void *args){
	struct Request *request;
	int thread_id = *((int*) args);

	// lock the request mutex
	pthread_mutex_lock(&request_mutex);

	while(1){
		if (clientRequests > 0) {
			request = GetRequests(&request_mutex);
			// Check if not null
			if (request){
				pthread_mutex_unlock(&request_mutex);
				// Handle the request here
				ClientConnectionsHandler(request, thread_id);
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
	struct Request *request;

	request = malloc(sizeof(struct Request));
	// Set values of request struct
	request->number = num_request;
	request->sockfd = socket_id;
	request->next = NULL;

	// Set the next value of the request
	pthread_mutex_lock(pthread_mutex);
	if (clientTotalRequests == 0){
		requests = request;
		last_request = request;
	} else {
		last_request->next = request;
		last_request = request;
	}
	clientTotalRequests++;

	// Unlock mutex and send signal
	pthread_mutex_unlock(pthread_mutex);
	pthread_cond_signal(pthread_cond_variable);

}

// Gets a request from the Queue
struct Request *GetRequests(pthread_mutex_t *pthread_mutex){
	struct Request *request;
	pthread_mutex_lock(pthread_mutex);

	if (clientTotalRequests > 0){
		request = requests;
		requests = requests->next;
		if (requests == NULL){
			last_request = NULL;

		}
		clientTotalRequests++;

	} else {
		// No clients in the Queue
		request = NULL;
	}

	// unlock the Mutex
	pthread_mutex_unlock(pthread_mutex);
	return request;


}



// TODO: DOESNT WORK AS EXPECTED
void HandleExitSignal() {
	// Close socket connection
	printf("\n\nClosing server and client sockets\n");
	shutdown(clientConnect, 1);
	shutdown(serverListen, 1);
	close (clientConnect);
	close (serverListen);

	// Kill threads and exit program
	printf("Killing threads, exiting program...\n");
	pthread_exit(&client_thread);

	// Exit program
	exit(0);
}

// Handle client connections
void* ClientConnectionsHandler(struct Request request, int socket_id) {
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

		read_size = ReceiveData(socket_id, message, MAXDATASIZE);

		fprintf(stderr, "Received username: %s\n", message);
		for(int i = 1; i < 12; i++){
			if(strcmp(message, loginDetails[i][0]) == 0){
				user = i;
			}
		}

		memset(message,0,sizeof(message));

		// Receive password
		read_size = ReceiveData(socket_id, message, MAXDATASIZE);

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
		resMes = SendData(socket_id, message, MAXDATASIZE);

	}

	printf("Client logged in successfully\n");
	// Await information on what the client wishes to do

	int clientFinished = 0;
	while(clientFinished != 1){
		ReceiveData(socket_id, message, MAXDATASIZE);

		if (strcmp("1", message) == 0){
	    // Start Minesweeper
			MinesweeperMenu(socket_id);
			printf("Ended, waiting for next command\n");
	  } else if (strcmp("2", message) == 0){
	    // Show Leaderboard
			printf("Sending leaderboard\n");
			SendLeaderboard(socket_id, leaderboard);
			printf("Sent leaderboard\n");
	  } else if (strcmp("3", message) == 0){
	    // Quit
			printf("Client Disconnecting\n");
	    close(socket_id);
			pthread_join(client_thread, NULL);
			clientFinished = 1;
	  }
	}

	// Generate Game State - TODO: Expand for multithreading
	//struct GameState gameState1;
}
