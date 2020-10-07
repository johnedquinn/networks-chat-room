CC = gcc
CFLAGS = -g -Wall -Werror -std=gnu99 -I../lib -fPIC
LD = gcc
LDFLAGS = -L../lib -lcrypto -lz

all: myclient

myclient:	client.o
	@echo "Linking $@"
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c pg3lib.h
	@echo "Compiling $@"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning..."
	rm -f $(all) *.o *.a

