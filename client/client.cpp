/*
Patrick Bald, John Quinn, Rob Reutiman
pbald, jquin13, rreutima
*/
using namespace std;
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
#include <iostream>

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
	int s = *(int*)arg;

    while(ACTIVE){
			char msg[BUFSIZ];

			if(recv(s, msg, sizeof(msg), 0) < 0){
				fprintf(stdout, "Error recieving message in client\n");
			}

			if(msg[0] == '\r') {
				/* Data Message */
				string m(msg);
				if(msg[1] == '0') {
					cout << "\n**** Incoming public message ****: " << endl;
					m.erase(0,2);
					cout << m;
  				fprintf(stdout, "> Please enter a command (BM: Broadcast Messaging, PM: Private Messaging,  EX: Exit)\n> ");
					fflush(stdout);
				} else if (msg[1] == '1') {
					cout << "\n**** Incoming private message ****: " << endl;
					m.erase(0,2);
					char* finalMessage = strdup(m.c_str());
					cout << decrypt(finalMessage);
					free(finalMessage);
  				fprintf(stdout, "> Please enter a command (BM: Broadcast Messaging, PM: Private Messaging,  EX: Exit)\n> ");
					fflush(stdout);
				}
			} else {
				return (void *) strdup(msg);
			}

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

	// Receive Acknowledgement
	short ack;
	if (recv(s, &ack, sizeof(ack), 0) < 0) {
		fprintf(stderr, "Unable to Receive Public Key\n");
		exit(1);
	}
	if (ntohs(ack) == 1) fprintf(stdout, "Existing User\n");
	else if (ntohs(ack) == 3) {
		fprintf(stdout, "User Already Logged In. Exiting Application.\n");
		exit(1);
	}
	else fprintf(stdout, "Creating New User\n");
	

	// Receive Server Public Key
	char skey [BUFSIZ];
	if (recv(s, skey, sizeof(skey), 0) < 0) {
		fprintf(stderr, "Unable to Receive Public Key\n");
		exit(1);
	}

	// Attempt Password Send
	ack = 2;
	while (ack != 1) {

		// Get User Password
		fprintf(stdout, "Enter password: ");
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
		if (recv(s, &ack, sizeof(ack), 0) < 0) {
			fprintf(stderr, "Unable to Receive Public Key\n");
			exit(1);
		}
		ack = ntohs(ack);
		if (ack > 1) {
			fprintf(stdout, "Incorrect Password. Please Try Again.\n");
		}
	}

	// Print Success
	fprintf(stdout, "Connection Established\n");
	
	// Generate Client Public Key
	char * ckey = getPubKey();

	// Send Public Key
	if (send(s, ckey, strlen(ckey) + 1, 0) < 0) {
		fprintf(stdout, "Unable to send username\n");
		exit(1);
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

	// Perform Log In and Sign Up
  login(s, username);


  // Make thread for handling messages
  pthread_t message_thread;
  int rc = pthread_create(&message_thread, NULL, handle_messages, &s);
	if(rc) {
		fprintf(stdout, "Error: unable to create thread\n");
		exit(-1);
	}

  fprintf(stdout, "> Please enter a command (BM: Broadcast Messaging, PM: Private Messaging,  EX: Exit)\n> ");
 	fflush(stdout);

  while(1){

    char operation[BUFSIZ];
    fgets(operation, BUFSIZ, stdin);

    if(!strncmp(operation, "PM", 2)){

		// Send private message to server
		char cmd[3] = "PM";
		if (send(s, cmd, 3, 0) < 0) {
			fprintf(stderr, "Unable to send BM operation\n");
			exit(1);
		}


		/* Get client list from server */
		void* clientList;
		pthread_join(message_thread, &clientList);
 		rc = pthread_create(&message_thread, NULL, handle_messages, &s);
		if(rc) {
			fprintf(stdout, "Error: unable to create thread\n");
			exit(-1);
		}
		string cl((char *) clientList);
		if (!cl.length()) {
			cout << "No Peers Online" << endl;
 			fprintf(stdout, "> Please enter a command (BM: Broadcast Messaging, PM: Private Messaging,  EX: Exit)\n> ");
			fflush(stdout);
			continue;
		}
		cout << "Peers online: " << endl;
		cout << cl;

		// Send target to server once verified as online
		char target[BUFSIZ] = "";
		char message[BUFSIZ] = "";

		cout << "Peer to message: "; fflush(stdout);

		int invalidTarget = 1;
		while(invalidTarget) {
			fgets(target, BUFSIZ, stdin);

			char * cl2 = strdup((char *) clientList);
			char *t = strtok(target, "\n");
			char *token = strtok((char*) cl2, "\n");

			while (token != NULL) {
				if(!strcmp(token, t)) {
					invalidTarget = 0;
					break;
				}
				token = strtok(NULL, "\n"); 
			}

			free(cl2);
			if(invalidTarget) {
				cout << "Invalid user. Please enter again: ";
				fflush(stdout);
			}

		}

		if(send(s, target, strlen(target) + 1, 0) < 0) {
			fprintf(stderr, "Unable to send BM operation\n");
			exit(1);
		}

		free(clientList);

		/* Get user public key from server */
		void *targetPubKey;
		pthread_join(message_thread, &targetPubKey);
 		rc = pthread_create(&message_thread, NULL, handle_messages, &s);
		if(rc) {
			fprintf(stdout, "Error: unable to create thread\n");
			exit(-1);
		}

		/* Get user message, encrypt and send */
		cout << "Enter Message: ";
		fgets(message, BUFSIZ, stdin);

		char* m = encrypt(message, (char*) targetPubKey);
		free(targetPubKey);

		if(send(s, m, strlen(m) + 1, 0) < 0) {
			fprintf(stderr, "Unable to send BM operation\n");
			exit(1);
		}

		/* Recieve confirmation of success or failure */
		void *confirmation;
		pthread_join(message_thread, &confirmation);
 		rc = pthread_create(&message_thread, NULL, handle_messages, &s);
		if(rc) {
			fprintf(stdout, "Error: unable to create thread\n");
			exit(-1);
		}
		cout << (char *) confirmation << endl;
		free(confirmation);

    } else if (!strncmp(operation, "BM", 2)){

			// Send broadcast message to server
			char cmd[3] = "BM";
			if (send(s, cmd, strlen(cmd) + 1, 0) < 0) {
				fprintf(stdout, "Unable to send BM operation\n");
				exit(1);
			}

			// recv acknowledgement from server
			void *tstatus;
			pthread_join(message_thread, &tstatus);
			if(pthread_create(&message_thread, NULL, handle_messages, &s) > 0){
				fprintf(stderr, "Unable to restart thread in BM\n");
				exit(-1);
			}

			// cout << "BM cmd ack: " << (char *) tstatus << endl;

			if(strcmp((char*)tstatus, "bm_cmd_ack")){
				fprintf(stderr, "Error receiving bm cmd ack\n");
				exit(-1);
			}

			free(tstatus);

			// get and send message to server
			fprintf(stdout, "Enter message: "); fflush(stdout);
			char msg[BUFSIZ];
			fgets(msg, BUFSIZ, stdin);

			if(send(s, msg, strlen(msg) + 1, 0) < 0){
				fprintf(stdout, "Error sending BM message to server\n");
				exit(1);
			}

			pthread_join(message_thread, &tstatus);
			if(pthread_create(&message_thread, NULL, handle_messages, &s) > 0){
				fprintf(stderr, "Unable to restart thread in BM\n");
				exit(-1);
			}

			// cout << "BM brd ack: " << (char*) tstatus << endl;

			if(strcmp((char*)tstatus, "bm_brd_ack")){
				fprintf(stderr, "Error receiving bm broadcast ack\n");
			}

			cout << "Public message sent." << endl;

			free(tstatus);
	
    } else if (!strncmp(operation, "EX", 2)){
			// Send broadcast message to server
			char cmd[3] = "EX";
			if (send(s, cmd, strlen(cmd) + 1, 0) < 0) {
				fprintf(stdout, "Unable to send BM operation\n");
				exit(1);
			}
			// Kill Thread
			pthread_cancel(message_thread);
			break;
    } else {
      fprintf(stdout, "Invalid input %s\n", operation);
    }

 		fprintf(stdout, "> Please enter a command (BM: Broadcast Messaging, PM: Private Messaging,  EX: Exit)\n> ");
		fflush(stdout);
  }

	pthread_join(message_thread, NULL);
  close(s);

  return 0;
}
