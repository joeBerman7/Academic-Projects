all: multi
multi:
	nasm -g -f elf32 -w+all -o multi.o multi.s
	gcc -g -m32 -Wall -o multi multi.o

clean:
	rm -f *.o multi