# include "multiboot.h"


/* Hardware text mode color constants. */
typedef char unit_8;
typedef unsigned short unit_16;
typedef unsigned long unit_32;
typedef unsigned long long unit_64;

enum vga_color
{
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

unit_16 x = 0;
unit_16 y = 0;

unit_16 strlen(char* str)
{
  unit_16 count;
  for (count = 0; str[count] != '\0'; count++);
  return count;
}

void clearScreen()
{
  unit_16 null = 0;
  unit_16* where = (unit_16*)0xB8000 + (y * 80 + x);
  *where = null;
}

void PrintChar(char c)
{
  /* set the backgroud color*/
  unit_16 backgroud = 0;
  unit_16 front= 15;
  unit_16 color = (backgroud << 4) | (front &0x0F);
  /* set position*/
  unit_16* where = (unit_16*)0xB8000 + (y * 80 + x);
  x++;
  *where = c | (color << 8);
}

void PrintString(char* str)
{
  int size = strlen(str);
  for (int i = 0; i < size; i++)
    PrintChar(str[i]);
}


void toString(char* str, unit_32 data)
{
	unit_32 temp = data;
	unit_32 base = 1;
	unit_32 quo;
	do
	{
		base *= 10;
		quo = temp / base;
	}while(quo > 0);
	
	base /= 10;
	
	for (int i = 0; base > 0; i++)
	{
		temp = data / base;
		str[i] = temp + '0';
		data -= temp * base;
		base /= 10; 
	}
	
}




void init( multiboot_info_t* pmb ) {

  memory_map_t *mmap;
  unit_32 memsz = 0;		/* Memory size in MB */
  static char memstr[10];

  for (mmap = (memory_map_t *) pmb->mmap_addr;
       (unsigned long) mmap < pmb->mmap_addr + pmb->mmap_length;
       mmap = (memory_map_t *) ((unsigned long) mmap
				+ mmap->size + 4 /*sizeof (mmap->size)*/)) {
        /* pmb->mmap_addr is a address or we could say the address of the 
        item in the Address Range Descriptor Structure buffer.
        In the item is a 20 Bytes number with contain the information
        of the , and first item gives the information of the information of
        the first block
        Actually 4 is the size of the type. A..something...I forget the name*/ 

    
    if (mmap->type == 1)	/* Available RAM -- see 'info multiboot' */
      memsz += mmap->length_low;
      // most like the E820, read when type equal to 1 and add the low length
      // because the memory not that large....
  }

  /* Convert memsz to MBs */
  memsz = (memsz >> 20) + 1;
  /* The + 1 accounts for rounding
	errors to the nearest MB that are
	in the machine, because some of the
	memory is othrwise allocated to
	multiboot data structures, the
	kernel image, or is reserved (e.g.,
	for the BIOS). This guarantees we
	see the same memory output as
	specified to QEMU.*/

  toString(memstr, memsz);
  // clean this screen. or we coudl say clean the buffer
  while(y < 24)
  {
	  while(x < 80)
	 {
		clearScreen();
		x++;
	 }
	 y++;
  }
  y=0;
  x=0;
  PrintString("MemOS: Welcome MemOS_2 System memory is: ");
  PrintString(memstr);
  PrintString("MB");
}

