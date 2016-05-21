all: MemOS_2

MemOS_2: boot.o init.o
	ld -T MemOS_2.ld -o MemOS_2 boot.o init.o

init.o:
	gcc -m32 -fno-builtin -fno-stack-protector -fno-strict-aliasing -fno-delete-null-pointer-checks -nostdinc -I. -g -Wall -std=c99 -c init.c -o init.o 

boot.o:
	gcc -c boot.S -o boot.o

install: MemOS_2
	cp MemOS_2 /mnt/Vdisk/boot

clean:
	-rm *.o MemOS_2
