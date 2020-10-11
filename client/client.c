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
#include <netdb.h>

#include "../lib/pg3lib.h"

#define MAX_LINE 4096

/* GLOBALS */
int EXIT = 0;
int ACTIVE = 1;

/*
* @func   handle_messages
* @desc   thread function for handling user messages
*/
void* handle_messages(void* arg){

    while(ACTIVE){
      //char message[BUFSIZ];

      // @todo recv message

    }

    return NULL;
}

/*
 * @func   login
 * @desc   Logs user in
 * --
 * @param  s         socket descriptor
 * @param  username  client username
 */
void login(int s, char* username){
	
	// Send Username
	if (send(s, username, sizeof(username), 0) < 0) {
		fprintf(stdout, "Unable to send username\n");
		exit(1);
	}

}

/*
* @func   main
* @desc   main driver, 
*         main thread to handle user input, server interaction
*/
int main(int argc, char* argv[]){

  // Get server name, port, uesrname
  char* host; int SERVER_PORT; char* username;
  if(argc == 4){
    host = argv[1];
	 SERVER_PORT = atoi(argv[2]);
    username = argv[3];
  } else {
    fprintf(stderr, "Usage: ./myserver [hostname] [port] [username]\n");
    exit(1);
  }

  /* Variables */
  struct hostent *hp;
  struct sockaddr_in sin;
  //char buf[MAX_LINE];
  int s;

  /* Translate host name into peer's IP address */
  hp = gethostbyname(host);
  if(!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }

  /* Build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

  /* Create Socket */
  if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket"); 
    exit(1);
  }

  /* Connect to server */
  printf("Connecting to %s on port %d\n", host, SERVER_PORT);

  if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("simplex-talk: connect");
    close(s); 
    exit(1);

  }

  while(!EXIT){

		// Perform Log In and Sign Up
    login(s, username);

    // Make thread for handling messages
    pthread_t message_thread;
    int rc = pthread_create(&message_thread, NULL, handle_messages, NULL);

    while(1){

      if(rc){
        fprintf(stdout, "Error: unable to create thread\n");
        exit(-1);
      }

      char option[BUFSIZ];
      fgets(option, BUFSIZ, stdin);

      if(!strncmp(option, "p", 1)){
        //private_message(); // @TODO write this func
      } else if (!strncmp(option, "b", 1)){
        //broadcast(); // @TODO write this func
      } else {
        fprintf(stdout, "Invalid input %s\n", option);
      }

    }

    close(s);

  }


  return 0;
}
