CC=gcc
CFLAGS=-Wall
hellomake: hellomake.o hellofunc.o
	$(CC) $(CFLAGS) -o hellomake hellomake.o hellofunc.o

hellomake.o: hellomake.c
	$(CC) $(CFLAGS) -c hellomake.c

hellofunc.o: hellofunc.c
	$(CC) $(CFLAGS) -c hellofunc.c

clean:
	rm -f *.o hellomake
	