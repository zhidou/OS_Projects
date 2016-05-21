
.globl _start
	.code16

_start: 
# Setup stack
	movw $0x9000, %ax
	movw %ax, %ss
	xorw %sp, %sp

# Setup DS
	# The BIOS loads the MBR at 0x7c00
	# I set this in the linker file 
	movw $0x0, %dx 
	movw %dx, %ds
	# Print string
	leaw msg, %si
	movw msg_len, %cx
1:
	lodsb
	# BIOS interupt function
	movb $0x0E, %ah
	int $0x10
	loop 1b



# probe memory using E820 
	# set es:di point the address 0
	movw $0x0, %dx
	movw %dx, %es
	movw %dx, %di
	# set all needed registar
	xor %ebx, %ebx
	xor %bp, %bp
	movl $0x0534D4150, %edx
	movw $0xE820, %ax
	movl $24, %ecx
	int $0x15

e820lp:
	# using dx to record the value of di and then we could 
	# visit the corresponding address by simply just add 
	# numbers to di, finaly we want to go to next 24 space
	# we should add 24 based on the original di
	movw %di, %dx 
	# the memory list structure is 8 8 4
	# last 4 bytes is type
	# di add 16 means go to the last byte directly
	addw $16, %di
	movw %di, %si
	# lod instruct only read what in si....
	lodsl
	cmp $0x1, %ax
	je addmem

ce820lp:

	movl $0x0534D4150, %edx
	movw $0xE820, %ax
	movw %dx, %di
	addw $20, %di
	movl $24, %ecx
	int $0x15

	test %ebx, %ebx
	jne e820lp
	jmp goon
	

addmem:

	movw %dx, %di
	# just read the first 32 bits of the second 8 bypes
	# because for we could not have such large memory.....that is 
	# tha fact..so 32 bits is large enough to tell us the size of 
	# memory. Second is is structure is little endian..
	# so just first 32 contains the information we want..
	addw $8, %di
	movw %di, %si
	lodsl
	shrl $20, %eax
	# for the use of stack.
	# first we put 0 in stack. and then we pop it and add the value 
	# of the size of this block and and push it..
	popw %cx
	addw %ax, %cx
	pushw %cx
	jmp ce820lp
	
goon:
	# reset all used registar for later use
	xor %eax, %eax
	xor %edx, %edx
	xor %ecx, %ecx

	popw %cx
	movw %cx, %ax
	shr $8, %ax
	call print

	movw %cx, %ax
	and $0xFF, %ax
	call print

	xor %ax, %ax
	leaw msg_unit, %si
	movw msg_unit_len, %cx

1:
	lodsb
	movb $0x0E, %ah
	int $0x10
	loop 1b

	ret

print: 
	# for the order, print first 4 bits and then last 4 bits
	pushw %dx
	movb %al, %dl
	shrb $4, %al
	cmpb $10, %al
	jge 1f
	addb $48, %al 
	#Add ASCII ‘0’ offset
	jmp 2f
1:	addb $55, %al 
	#Add ASCII ‘A’-10 offset
2:	movb $0x0E, %ah 
	#command in the BIOS to print the ASCII value of 
	int $0x10

	movb %dl, %al
	andb $0x0F, %al
	cmpb $10, %al
	jge 1f
	addb $48, %al
	jmp 2f
1:	addb $55, %al
2:	movb $0x0E, %ah
	int $0x10
	popw %dx
	ret

msg:	.asciz "Memo-1: Welcome Memo-1 System Memory is: "
msg_len:.word . - msg
msg_unit: .asciz " MB"
msg_unit_len: .word . - msg_unit


	.org 0x1FE
	.byte 0x55
	.byte 0xAA
