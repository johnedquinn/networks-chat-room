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

/* GLOBALs */
int NUM_THREADS = 0;

/*
* @func   client_interaction
* @desc   thread function for handling user messages
*/
void* client_interaction(void* arg){



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

  // @TODO set up socket for connections


  while((client_sock = accept(s.get_s(), (struct sockaddr *)$s.s_in, &s_inlen)) > 0 ){
    
    if(NUM_THREADS == 10){
      fprint(stdout, "Connection Refused: Max clients online.\n");
      continue;
    }

    // Create new thread for each accepted client
    pthread_t user_thread;
    struct user_thread_args args;
    NUM_THREADS++;
    if(pthread_create(&user_thread, NULL, client_interaction, (void*)&args) < 0){
      perror("Error creating user thread\n");
      return 1;
    }

  }


  return 0;
}