all: task1 task4
task1:
	gcc -m32 -g -Wall -o hexeditplus task1.c
task4:
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c
	
.PHONY: clean
clean:
	rm -f hexeditplus task4