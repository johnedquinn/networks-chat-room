CC = g++
CFLAGS = -g -Wall -Werror -std=c++11 -I../lib -fPIC
LD = g++
LDFLAGS = -L../lib -lcrypto -lz -pthread

all = myserver

myserver: server.o
	@echo "Linking $@"
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.cpp pg3lib.h
	@echo "Compiling $@"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning..."
	rm -f $(all) *.o *.a
