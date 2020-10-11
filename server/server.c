/*
Patrick Bald, John Quinn, Rob Reutiman
pbald, jquin13, rreutima
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../lib/pg3lib.h"

#define MAX_LINE 4096
#define MAX_THREAD 10

/* GLOBALs */
int NUM_THREADS = 0;

/*
* @func   client_interaction
* @desc   thread function for handling user messages
*/
void* client_interaction(void* arg){

	int len;
  char username[MAX_LINE] = "";
	char command[MAX_LINE] = "";
	int client_sock = *(int*)arg;

	/* Get username */
	if(recv(client_sock, username, sizeof(username), 0) < 0) {
		perror("Server Received Error!"); 
		exit(1);
	}

	printf("Username: %s\n", username);

	/* Loop to get commands */
	while(1) {

		if((len = recv(client_sock, command, sizeof(command), 0)) == -1) {
			perror("Server Received Error!"); 
			exit(1);
		}
  	if(len == 0) break;

		printf("%s\n", command);

		/* Command specific functions */
		if(!strncmp(command, "BM", 2)) {

		} else if(!strncmp(command, "PM", 2)) {

		} else if(!strncmp(command, "EX", 2)) {

		}

		bzero((char*)command, sizeof(command));

	}
  return NULL;
}

/*
* @func   main
* @desc   main driver, 
*         main thread for socket listener for incoming connections
*/
int main(int argc, char* argv[]){

  // Get port number
  int port;
  if(argc == 2){
    port = atoi(argv[1]);
  } else {
    fprintf(stderr, "Usage: ./myserver [PORT]\n");
    exit(1);
  }

  // Set up socket for connections
  struct sockaddr_in sin, client_addr;
  int s, client_sock;

  // Build Address Data Structure
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	
	// Set passive option 
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("myftpd: socket");
		exit(1);
	}
	
	// Set Socket Option
	int opt = 1;
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(int))) < 0) {
		perror("myftpd: setsocket"); 
		exit(1);
	}

	// Bind Socket
	if((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("myftpd: bind"); 
		exit(1);
	}

	// Listen
	if((listen(s, MAX_THREAD)) < 0) {
		perror("myftpd: listen"); 
		exit(1);
	} 
	
	/* wait for connection, then receive and print text */
	socklen_t addr_len = sizeof(client_addr);
	printf("Waiting for connections on port %d\n", port);

  while((client_sock = accept(s, (struct sockaddr *)&client_addr, &addr_len)) < 0){
    
    if(NUM_THREADS == 10){
      fprintf(stdout, "Connection Refused: Max clients online.\n");
      continue;
    }

    // Create new thread for each accepted client
    pthread_t user_thread;
    NUM_THREADS++;
    if(pthread_create(&user_thread, NULL, client_interaction, &client_sock) < 0){
      perror("Error creating user thread\n");
      return 1;
    }

  }

  return 0;
}
