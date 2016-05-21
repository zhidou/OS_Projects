#include "types.h"

extern void interrupt_handler();
extern void schedule();
extern void set_IDT(int num, uint32_t offset, uint16_t selector, uint8_t flags);

uint32_t tick = 0;

void time_interupt_handler()
{
	tick += 3;
	outb(0x20 , 0x20);
	schedule();
}

uint32_t getTick()
{
	return tick;
}

void init_timer(uint16_t frequency)
{
	// 0x8e I think is the most important one, when setting IDT
	// in this code, it contains information of privilege level, 
	// which is 0 and e = 1110 which means interrupt gate
	set_IDT(32, (uint32_t)interrupt_handler, 0x08, 0x8e);
	uint16_t divisor = 1193180 / frequency;
	// Send the command port, 0x36 meanx Chanel 0, l/d access, mode 2
	outb(0x43, 0x34);
	// the port just have 8 bits
	uint8_t low = (uint8_t)(divisor & 0xff);
	uint8_t high = (uint8_t)((divisor >> 8) & 0xff);
	// send frequency divisor to data port of chanel 0
	outb(0x40, low);
	outb(0x40, high);
}


