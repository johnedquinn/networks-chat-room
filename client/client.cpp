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
#include <string>

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
      string msg;
		if(recv(arg.s, msg, sizeof(msg), 0) < 0){
			fprintf(stdout, "Error recieving message in client\n");
		}

		fprintf(stdout, "Message: %s", msg);
		fflush(stdout);

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
	if (send(s, username, strlen(username) + 1, 0) < 0) {
		fprintf(stdout, "Unable to send username\n");
		exit(1);
	}

	// Receive Server Public Key
	char skey [BUFSIZ];
	if (recv(s, skey, sizeof(skey), 0) < 0) {
		fprintf(stderr, "Unable to Receive Public Key\n");
		exit(1);
	}
	fprintf(stdout, "Received Server Key: %s\n", skey);

	// Get User Password
	fprintf(stdout, "Password: ");
	char pass [BUFSIZ];
	fgets(pass, sizeof(pass), stdin);

	// Encrypt Password
	char * epass = encrypt(pass, skey);

	// Send Encrypted Password
	if (send(s, epass, strlen(epass) + 1, 0) < 0) {
		fprintf(stdout, "Unable to send username\n");
		exit(1);
	}

	// Receive Acknowledgement
	short ack;
	if (recv(s, &ack, sizeof(ack), 0) < 0) {
		fprintf(stderr, "Unable to Receive Public Key\n");
		exit(1);
	}
	if (ntohs(ack) < 0) {
		fprintf(stdout, "Unable to Login or Create Account");
	}
	
	// Generate Client Public Key
	char * ckey = getPubKey();

	// Send Public Key
	if (send(s, ckey, strlen(ckey) + 1, 0) < 0) {
		fprintf(stdout, "Unable to send username\n");
		exit(1);
	}

}

/*
 * @func Broadcast Message (BM)
 *
 * @params s: socket descriptor for server
 */
void BM(int s){

	// Send broadcast message to server
	if (send(s, "BM", 3, 0) < 0) {
		fprintf(stdout, "Unable to send BM operation\n");
		exit(1);
	}

	// recv acknowledgement from server
	short ack;
	if (recv(s, &ack, sizeof(ack), 0) < 0) {
		fprintf(stderr, "Unable to Receive Public Key\n");
		exit(1);
	}
	if (ntohs(ack) < 0) {
		fprintf(stdout, "Failed BM confirmation\n");
	}

	// get and send message to server
	fprintf(stdout, "Enter message: "); fflush(stdout);
	string msg;
	getline(cin, msg);

	if(send(s, msg, msg.length() + 1, 0) < 0){
		fprintf(stdout, "Error sending BM message to server\n");
		exit(1);
	}
	

	// recv confirmation
	if (recv(s, &ack, sizeof(ack), 0) < 0) {
		fprintf(stderr, "Unable to receive server sent confirmation\n");
		exit(1);
	}
	if (ntohs(ack) < 0) {
		fprintf(stdout, "Failed BM confirmation\n");
	}

	

}

typedef struct args args;
struct args {
	int s;
};


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

  printf("Connection established\n>");

  while(!EXIT){

		// Perform Log In and Sign Up
    login(s, username);

	 args a;
	 a.s = s;

    // Make thread for handling messages
    pthread_t message_thread;
    int rc = pthread_create(&message_thread, NULL, handle_messages, args);

    while(1){

      if(rc){
        fprintf(stdout, "Error: unable to create thread\n");
        exit(-1);
      }

      char operation[BUFSIZ];
      fgets(operation, BUFSIZ, stdin);

      if(operation == "PM"){
			PM(s);
      } else if (operation == "BM"){
        BM(s);
      } else {
        fprintf(stdout, "Invalid input %s\n", option);
      }

		fprintf(stdout, "\n> "); fflush(stdout);

    }

    close(s);

  }


  return 0;
}
