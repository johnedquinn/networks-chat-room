## Project Overview
This project aims to create a chat application by manipulating TCP. 

### Project Members
Patrick Bald (pbald)  
John Quinn (jquinn13)  
Rob Reutiman (rreutima)  

## Project Structure
- `server`: Directory containing server source code, makefile, and example files
- `client`: Directory containing client source code, makefile, and example files
- `server/server.cpp`: Source code for server
- `server/authfile.txt`: File containing the usernames and passwords of users
- `client/client.cpp`: Source code for client

## To Run
```terminal
foo@comp1 $ cd server
foo@comp1 $ make
foo@comp1 $ ./chatserver $(PORT)

foo@comp2 $ cd client
foo@comp2 $ make
foo@comp2 $ ./chatclient $(COMP1) $(PORT) $(USERNAME)
```

## Example Commands
```terminal
foo@comp2 $ BM
foo@comp2 $ PM
foo@comp2 $ EX
```
