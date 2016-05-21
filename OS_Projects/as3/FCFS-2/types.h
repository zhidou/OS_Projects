#define FALSE 0
#define TRUE 1
#define N 3
#define _disable_interrupt() __asm__ volatile("cli\n\t");
#define _enable_interrupt() __asm__ volatile("sti\n\t");
#define NULL 0
#define FIFO 0
#define SRT 1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;

#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

void outb (uint16_t port, uint8_t value);
