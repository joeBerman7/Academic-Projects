all:myshell mypipe

myshell:	LineParser.o	myshell.o
	gcc	-g	-m32	-Wall	 -o	myshell myshell.o LineParser.o

myshell.o:	myshell.c	LineParser.h
	gcc	-g	-m32	-Wall	 -c	-o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall	-c	-o	LineParser.o	LineParser.c

mypipe:	mypipe.o
	gcc -g -m32 -Wall	-o	mypipe	mypipe.o

mypipe.o: mypipe.c
	gcc -g -m32 -Wall	-c	-o	mypipe.o	mypipe.c


.PHONY: clean

clean:
	rm -f *.o *.txt	looper	myshell	mypipe