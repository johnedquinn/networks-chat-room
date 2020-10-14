CC = g++ 
CFLAGS = -g -Wall -Werror -std=c++11 -fPIC
LD = g++
LDFLAGS = -L../lib -lcrypto -lz -pthread

all = myserver

myserver: server.o
	@echo "Linking $@"
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.cpp ../lib/pg3lib.h 
	@echo "Compiling $@"
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	@echo "Cleaning..."
	rm -f $(all) *.o *.a
