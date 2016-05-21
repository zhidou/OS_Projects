.global stack

.bss
.align 0x1000
# allocate 4KB space for stack
.comm stack, 0x1000

.data
.align 0x4

gdt:
    # The first entry of GDT is NULL    
    .long 0
    .long 0

    # The second entry of GDT 
    # Because we use basic flat model so the base address and segment limit
    # of both text section and data section are the same. 
    # Here the base address is 0 and segment limit is 4GM
    # the first one actully is lower bits
    # A in the second line means Execute/Read, namely this is setting text segment
    .long 0x0000FFFF
    .long 0x00CF9A00

    # 2 in the second line means Read/Write, namely this is setting data segement.
    .long 0x0000FFFF
    .long 0x00CF9200

# This is the information should be loaded in GDTR
gdt_ptr:
    # This is the limitation of GDT
    .short 0x7FF
    # This is the base address of GDT. Actually, we pick the segment selector from 
    # IDT and add this base address we could find the segment in GDT
    .long gdt

.text
.global _start
.global loop
.global switch_to
.global interrupt_handler

_start:
    # for GRAB multiboot check
    jmp real_start
    .align 4
    .long 0x1BADB002
    .long 0x00000003
    .long 0xE4524FFB

real_start:
    # load GDT information to GDTR
    lgdt gdt_ptr
    # we jump to test segment. The segment selector has 16 bits
    # First 3 for choose GDT and privilege level. And text segment
    # is our 1 entry (0 entry is NULL), thus the relative address is 0x08
    ljmp $0x08 , $1f

1:
    movw $0x10 , %ax
   
    movw %ax , %ds 
    movw %ax , %ss
    movw %ax , %es
    movw %ax , %fs
    movw %ax , %gs

    # set the stack pointer to the top of the stack
    movl $stack+0x1000, %esp

    # save multiboot parameter, for eventual call to C
    pushl %ebx
    call init

    cli 
    hlt
loop:
    jmp loop

