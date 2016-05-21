#include "types.h"

void outb (uint16_t port, uint8_t value)
{
    __asm__ __volatile__("outb %0, %1" :: "a"(value), "Nd"(port));
    return;
}
/* 0x20 is the command port of master pic
 * 0x21 is the data port of master pic
 * 0xA0 is the command port of slave pic
 * 0xA1 is the data port of master pic
 */
void init_PIC(void)
{
	outb (0x20, 0x11); // initialize master pic
	outb (0x21, 0x20); // reset the beginning vector
	outb (0x21, 0x04); // tell master the slave
	outb (0x21, 0x01); // environment set

	// initialize slave
	outb (0xA0, 0x11); // initialize slave pic
	outb (0xA1, 0x28); // reset the beginning vector
	outb (0xA1, 0x02); // tell slave the identity
	outb (0xA1, 0x01); // environment set

	// unmask interrupts we will use (clear bit)
  	outb (0x21, 0xfe); // timer (bit 0)
  	outb (0xA1, 0xff);
}