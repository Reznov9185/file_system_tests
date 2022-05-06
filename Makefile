# *-* Makefile *-*

all: fileops

fileops: fileops.o
	g++ fileops.o -o fileops -lm
	
fileops.o: fileops.cpp
	g++ -c fileops.cpp
clean:
	rm -f *.o
	rm -f *b
