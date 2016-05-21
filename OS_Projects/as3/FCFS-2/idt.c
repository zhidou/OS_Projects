# include "types.h"
extern void time_interupt_handler();

typedef struct
{
	uint16_t lowOffset;			// lower bits of Offset of interupt handler
	uint16_t SSelector;			// Segment selector for GDT(basically a offset)
	uint8_t reserved;			// reserved space. According to dev_so they are always 0
	uint8_t flags;				// flags and a reserved area
	uint16_t highOffset;		// higher bits of offset of interupt handler
}IDT_entry_table;

typedef struct 
{
	uint16_t limit;				// size of IDT
	uint32_t base;				// base address of IDT
}IDT_ptr_strcut;

static IDT_entry_table IDT[33];
IDT_ptr_strcut IDT_ptr;

void default_interupt_handler()
{
	//
}


void set_IDT(int num, uint32_t offset, uint16_t selector, uint8_t flags)
{
	IDT[num].lowOffset = (uint16_t)(offset & 0xffff);
	IDT[num].highOffset = (uint16_t)((offset >> 16) & 0xffff);
	IDT[num].SSelector = selector;
	IDT[num].reserved = 0;
	IDT[num].flags = flags;
}

void initIDT()
{
	// because the first block is 0, so minus 1
	IDT_ptr.limit = sizeof(IDT_entry_table) * 33 - 1;
	IDT_ptr.base = (uint32_t)(&IDT[0]);
	uint64_t IDTR = (uint64_t)IDT_ptr.base << 16 | IDT_ptr.limit;
	// 0x8e I think is the most important one, when setting IDT
	// in this code, it contains information of privilege level, 
	// which is 0 and e = 1110 which means interrupt gate
	for (int i = 0; i < 32; i++)
	{
		set_IDT(i, (uint32_t)default_interupt_handler, 0x08, 0x8e);
	}
	// load IDT
	__asm__ __volatile__ ("lidt %0" : : "m" (IDTR)); 
}