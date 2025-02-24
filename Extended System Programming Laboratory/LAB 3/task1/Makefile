all: task1

task1: start.o util.o
	ld -m elf_i386 -s -o task1 start.o util.o

start.o: start.s
	nasm -f elf32 -o start.o start.s

util.o: util.c
	gcc -m32 -c -nostdlib -fno-stack-protector util.c -o util.o

.PHONY: clean

clean:
	rm -f *.o task1

