.global interrupt_handler
.align 4

interrupt_handler:

	pushal

	movw %ds , %ax
	pushl %eax

	movw $0x10 , %ax
	movw %ax , %ds
	movw %ax , %es
	movw %ax , %fs
	movw %ax , %gs
	
	call time_interupt_handler

	popl %ebx
	movw %bx , %gs
	movw %bx , %fs
	movw %bx , %es
	movw %bx , %ds

	popal

	iret