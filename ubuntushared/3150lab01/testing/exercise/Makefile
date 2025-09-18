CC=gcc
CFLAGS=-Wall
main: main.o foo.o bar.o
	$(CC) $(CFLAGS) -o lab1 main.o foo.o bar.o
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
foo.o: foo/foo.c
	$(CC) $(CFLAGS) -c foo/foo.c
bar.o: bar/bar.c
	$(CC) $(CFLAGS) -c bar/bar.c
clean:
	rm *.o lab1
