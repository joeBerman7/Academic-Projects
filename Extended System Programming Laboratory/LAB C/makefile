myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

LineParser.o:  LineParser.c  LineParser.h
	gcc -g -Wall -m32 -c -o LineParser.o LineParser.c

mypipeline: mypipeline.c
	gcc -g -Wall -m32 -o mypipeline mypipeline.c