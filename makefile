CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lm

ping: ping.o
	$(CC) $(CFLAGS) -o ping ping.o $(LDFLAGS)

ping.o: ping.c ping.h
	$(CC) $(CFLAGS) -c ping.c

clean:
	rm -f ping.o ping
