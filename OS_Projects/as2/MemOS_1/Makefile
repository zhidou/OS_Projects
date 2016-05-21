all: MemOs_1_test

MemOs_1.o: MemOs_1.s
	as --32 MemOs_1.s -o MemOs_1.o

MemOs_1: MemOs_1.o
	ld -T MemOs_1.ld MemOs_1.o -o MemOs_1

MemOs_1_test: MemOs_1
	dd bs=1 if=MemOs_1 of=MemOs_1_test count=512

install:
	dd if=MemOs_1 of=/root/disk.img bs=512 count=1

clean:
	rm MemOs_1.o MemOs_1 MemOs_1_test
