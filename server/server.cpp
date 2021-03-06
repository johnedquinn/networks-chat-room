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
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <utility>

#include "../lib/pg3lib.h"

using namespace std;

#define MAX_THREAD 10

/* GLOBALs */
int NUM_THREADS = 0;
char AUTH_FILE [30] = "authfile.txt";

typedef struct args args;
struct args {
		int s;
		pthread_mutex_t lock;
		char* username;
		map<string, pair<int, string> > * activeUsers;
};

/*
 * @func   client_authenticate
 * @desc   logs in user or creates account
 * --
 * @param  s  socket desc
 */
string client_authenticate (args * a, char uname []) {
	// Grab socket
	int s = a->s;

	// Make Sure User Is Not Already Logged In
	string u(uname);
	for (auto e : *(a->activeUsers)) {
		if (e.first == u) {
			short ack = 3; ack = htons(ack);
			if (send(s, &ack, sizeof(ack), 0) < 0) {
				fprintf(stderr, "Unable to send ack\n");
				exit(1);
			}
			u = "";
			return u;
		}
	}

	// Open Authentication File
	FILE * fp = fopen(AUTH_FILE, "r");
	if (!fp) {
		fprintf(stderr, "Unable to open Auth File\n");
		exit(1);
	}

	// Loop Through File
	char fline [BUFSIZ];
	char *fuser = NULL; char *fpass = NULL;
	int userFound = 0;
	while (fgets(fline, sizeof(fline), fp)) {
		fuser = strtok(fline, "\r");
		if (fuser) fpass = strtok(NULL, "\n");
		if (!strcmp(fuser, uname)) {
			userFound = true;
			break;
		}
		bzero((char *)&fline, sizeof(fline));
	}
	fclose(fp);

	// Send Acknowledgement (1:Login, 2:AccountCreation)
	short ack;
	if (userFound) {
		ack = htons(1);
		if (send(s, &ack, sizeof(ack), 0) < 0) {
			fprintf(stdout, "Unable to send ack\n");
			exit(1);
		}
	} else {
		ack = htons(2);
		if (send(s, &ack, sizeof(ack), 0) < 0) {
			fprintf(stdout, "Unable to send ack\n");
			exit(1);
		}
	}
	
	// Generate Server Public Key
	char * skey = getPubKey();
	
	// Send Public Key
	if (send(s, skey, strlen(skey) + 1, 0) < 0) {
		fprintf(stdout, "Unable to send server public key\n");
		exit(1);
	}

	ack = 2;
	while (ack != 1) {

		// Receive Client Password
		char epass [BUFSIZ];
		if (recv(s, epass, sizeof(epass), 0) < 0) {
			fprintf(stderr, "Unable to get client username\n");
			exit(1);
		}

		// Decrypt Password
		char * pass = decrypt(epass);
		if (pass[strlen(pass)-1] == '\n') pass[strlen(pass)-1] = '\0';

		// Login
		if (userFound) {
			if (!strcmp(fpass, pass)) ack = 1;
			else ack = 2;
		
		// Create Account
		} else {
			ofstream out;
			out.open(AUTH_FILE, std::ios::app);
			out << uname << "\r" << pass << "\n";
			out.close();
			ack = 1;
		}

		// Send Acknowledgement
		short tack = htons(ack);
		if (send(s, &tack, sizeof(tack), 0) < 0) {
			fprintf(stdout, "Unable to send server public key\n");
			exit(1);
		}
	}

	// Receive Client Public Key
	char ckey [BUFSIZ];
	if (recv(s, ckey, sizeof(ckey), 0) < 0) {
		fprintf(stderr, "Unable to get client pubkey\n");
		exit(1);
	}
	
	string cpub(ckey);
	return cpub;

}



/*
* @func   pm
* @desc   private message handler function
*/

void pm(args *a) {

	char target[BUFSIZ] = "";
	char message[BUFSIZ] = "";

	/* Send list of active users */
	string names = "";
	string username(a->username);
	for(auto &key: *(a->activeUsers)) {
		if (key.first != username) names.append(key.first + "\n");
	}

	if(send(a->s, names.c_str(), names.length() + 1, 0) < 0) {
		perror("Error sending client list."); 
		exit(1);
	}

	// No Names Found
	if (!names.length()) return;

	/* Recieve username of target user */
	if(recv(a->s, target, sizeof(target), 0) < 0) {
		perror("Error receiving username."); 
		exit(1);
	}

	string t(target);

	/* Reply with public key */
	const char* pubKey = (a->activeUsers->at(t).second).c_str();

	if(send(a->s, pubKey, strlen(pubKey) + 1, 0) < 0) {
		perror("Error sending target public key to client.\n");
		exit(1);
	}

	/* Recieves encrypted message */
	if(recv(a->s, message, sizeof(message), 0) < 0) {
		perror("Error receiving username."); 
		exit(1);
	}


	/* Sends message to user socket if online */
	string m(message);
	m = "\r1" + m;
	const char* finalMessage = m.c_str();

	try {
		if(send(a->activeUsers->at(t).first, finalMessage, strlen(finalMessage) + 1, 0) < 0) {
			perror("Error sending message to user.\n");
			exit(1);
		}

		/* Sends confirmation of failure to client */
		char success[BUFSIZ] = "Private message sent.";
		if(send(a->s, success, strlen(success) + 1, 0) < 0) {
			perror("Error sending confirmation to client.\n");
			exit(1);
		}
	} catch ( out_of_range& oor2) {
		/* Sends confirmation of failure to client */
		char failure[BUFSIZ] = "Private message failed to send.";
		if(send(a->s, failure, strlen(failure) + 1, 0) < 0) {
			perror("Error sending confirmation to client.\n");
			exit(1);
		}

	}


}

void bm(args* a){

	// fprintf(stdout, " Running bm func\n");

	// send acknowledgement for bm
	char bm_ack[11] = "bm_cmd_ack";
	if(send(a->s, bm_ack, strlen(bm_ack) + 1, 0) < 0){
		fprintf(stdout, "Error sending bm acknowledgement\n");
	}

	// get message
	char buf[BUFSIZ];
	if(recv(a->s, buf, sizeof(buf), 0) < 0){
		fprintf(stdout, "Error recieving bm message\n");
	}

	string msg(buf);
	msg = "\r0" + msg;

	// send message to all active users
	
	for ( auto user : *(a->activeUsers)){

		// fprintf(stdout, "iterating through users\n");
		// fprintf(stdout, "socket descriptor: %d\n", user.second.first);

		if(user.second.first == a->s) continue;

		if(send(user.second.first, msg.c_str(), strlen(msg.c_str()) + 1, 0) < 0){
			fprintf(stderr, "Error broadcasting message\n");
		}

	}

	// send acknowledgement for bm broadcast
	char bm_broadcast_ack[11] = "bm_brd_ack";
	if(send(a->s, bm_broadcast_ack, strlen(bm_broadcast_ack) + 1, 0) < 0){
		fprintf(stdout, "Error sending bm broadcast acknowledgement\n");
	}

}


/*
* @func   cleanup
* @desc   closes socket, updates activeUsers
* --
* @param  a
*/
void cleanup(args * a) {

	// Close Socket
	close(a->s);

	// Update Active Users
	pthread_mutex_lock(&(a->lock));
	a->activeUsers->erase(a->username);
  NUM_THREADS--;
	pthread_mutex_unlock(&(a->lock));

	free(a->username);
	
}


/*
* @func   client_interaction
* @desc   thread function for handling user messages
*/
void* client_interaction(void* arguments){

	int len;
	char command[BUFSIZ] = "";
	args *a = (args*)arguments;
	
	pthread_mutex_lock(&(a->lock));
  NUM_THREADS++;
	pthread_mutex_unlock(&(a->lock));

	// Receive Client Username
	char username [BUFSIZ];
	if(recv(a->s, username, sizeof(username), 0) < 0) {
		fprintf(stderr, "Unable to get client username\n");
		exit(1);
	}
	string uname(username);
	a->username = strdup(username);

	// Login or Create User
	/* @TODO: Verify user isn't already online */
	string cpub = client_authenticate(a, username);
	if (!cpub.length()) return NULL;

	pthread_mutex_lock(&(a->lock));
	a->activeUsers->insert(pair<string, pair<int, string> >(uname, pair<int, string> (a->s, cpub)));
	pthread_mutex_unlock(&(a->lock));

	/* Loop to get commands */
	while(1) {

		if((len = recv(a->s, command, sizeof(command), 0)) == -1) {
			perror("Server Received Error!"); 
			exit(1);
		}
  	if(len == 0) break;

		/* Command specific functions */
		if(!strncmp(command, "BM", 2)) {
			bm(a);
		} else if(!strncmp(command, "PM", 2)) {
			pm(a);
		} else if(!strncmp(command, "EX", 2)) {
			cleanup(a);
			break;
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
		perror("myserver: socket");
		exit(1);
	}
	
	// Set Socket Option
	int opt = 1;
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(int))) < 0) {
		perror("myserver: setsocket"); 
		exit(1);
	}

	// Bind Socket
	if((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("myserver: bind"); 
		exit(1);
	}

	// Listen
	if((listen(s, MAX_THREAD)) < 0) {
		perror("myserver: listen"); 
		exit(1);
	} 
	
	/* wait for connection, then receive and print text */
	socklen_t addr_len = sizeof(client_addr);
	printf("Waiting for connections on port %d\n", port);

	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	map<string, pair<int, string> > activeUsers;

  while(1) {

		if((client_sock = accept(s, (struct sockaddr *)&client_addr, &addr_len)) < 0){
			perror("myserver: accept"); 
			exit(1);
		}

 		pthread_mutex_lock(&lock);
    if(NUM_THREADS == 10){
      fprintf(stdout, "Connection Refused: Max clients online.\n");
      continue;
    }
		pthread_mutex_unlock(&lock);

    // Create new thread for each accepted client
   	pthread_t user_thread;

		args *a = (args *) calloc((size_t)1, sizeof(args));
		a->s = client_sock;
		a->lock = lock;
		a->activeUsers = &activeUsers;

    if(pthread_create(&user_thread, NULL, client_interaction, (void*)a) < 0){
      perror("Error creating user thread\n");
      return 1;
    }

  }

  return 0;
}
