CC=gcc
CFLAGS=-Wall

all: sender receiver

sender: sender.c common.h
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c common.h
	$(CC) $(CFLAGS) -o receiver receiver.c

clean:
	rm -f sender receiver
