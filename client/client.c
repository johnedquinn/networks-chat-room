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

#include "lib/pg3lib.h"

/* GLOBALS */
int EXIT = 0;
int ACTIVE = 1;

/*
* @func   handle_messages
* @desc   thread function for handling user messages
*/
void* handle_messages(void* arg){

    while(ACTIVE){
      char message[BUFSIZ];

      // @todo recv message

    }

    return NULL;
}

void login(char* username){

}

/*
* @func   main
* @desc   main driver, 
*         main thread to handle user input, server interaction
*/
int main(int argc, char* argv[]){

  // Get server name, port, uesrname
  char* host; int port; char* username;
  if(argc == 4){
    host = argv[1];
    port = atoi(argv[2]);
    username = argv[3];
  } else {
    fprintf(stderr, "Usage: ./myserver [hostname] [port] [username]\n");
    exit(1);
  }

  while(!EXIT){
    login(username);

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
        private_message(); // @todo write this func
      } else if (!strncmp(option, "b", 1)){
        broadcast(); // @todo write this func
      } else {
        fprintf(stdout, "Invalid input %s\n", options);
      }

    }

    s.close_socket();

  }


  return 0;
}