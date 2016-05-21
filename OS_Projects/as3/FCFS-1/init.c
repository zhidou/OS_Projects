#include "multiboot.h"
#include "types.h"
#include "fun.h"
#define N 3
#define NULL 0
#define FIFO 1
#define SRT 0

// Structure of thread
typedef struct 
{
  uint32_t *sp;
  uint32_t (*task)();
  uint32_t TID;
  uint32_t *bp;
  uint32_t flag;
  uint32_t status;
  uint32_t finished;
  uint32_t runningT;
  uint32_t comeT;
  uint32_t *priority;
}TCB;

// stack of each thread
static uint32_t stack1[1024];
static uint32_t stack2[1024];
static uint32_t stack3[1024];

// thread pool
TCB thread[N];
TCB *curThread = NULL;
/* TID LinkedList, actuall we use TID queue to control
   the generation of thread
 */
typedef struct
{
  uint32_t TID;
  int used;
}TIDQ;

TIDQ tidq[N];

/* The ready heap*/
TCB *readyHeap[N];
int heapLenght = 0;

void HeapAjust(int node)
{
  TCB *temp = readyHeap[node];
  int i;
  for (i = 2 * node + 1; i < heapLenght; i *= 2)
  {
    if (i + 1 < heapLenght && *(readyHeap[i]->priority) > *(readyHeap[i + 1]->priority))
      i++;
    if (!(*(readyHeap[node]->priority) > *(readyHeap[i]->priority))) break;
    readyHeap[node] = readyHeap[i];
    node = i;
  }
  readyHeap[node] = temp;
}

void initHeap()
{
  int i;
  for (i = (heapLenght-1-1)/2; i >= 0; i--)
    HeapAjust(i);
}


TCB * next_thread()
{
  TCB *temp = readyHeap[0];
  readyHeap[0] = readyHeap[heapLenght - 1];
  readyHeap[heapLenght - 1] = temp;
  if (heapLenght > 2)
  {
    heapLenght--;
    HeapAjust(0);
    heapLenght++;
  }
  return readyHeap[heapLenght - 1];
}

void schedule()
{

  if (!heapLenght)
  {
    terminal_writestring("No more tasks!");
    __asm__ volatile("jmp loop\n\t");
  }

  TCB *next = next_thread();

  TCB *temp = curThread;
  curThread = next;
  __asm__ volatile("call switch_to\n\t"::"S"(temp), "D"(next));
}


void updatePrioriy()
{
  if (FIFO)
    curThread->comeT += N;
}


void yield()
{
  if (!curThread->runningT)
  {
    curThread->finished = 1;
    *(curThread->priority) = NULL;
    heapLenght--;
    tidq[curThread->TID].used = 0;
    printID(curThread->TID + 1);
    terminal_writestring("is done!");
    Enter();
  }
  else updatePrioriy();
  schedule();
}




void task1()
{
  while (thread[0].runningT)
  { 
    terminal_writestring("Running thread: <1>");
    Enter();
    for (float i = 0; i < 100000; i +=0.01){};
    thread[0].runningT--;
    yield();
  }
}

void task2()
{
  while (thread[1].runningT)
  {
    terminal_writestring("Running thread: <2>");
    Enter();
    for (float i = 0; i < 100000; i +=0.01){};
    thread[1].runningT--;
    yield();
  }
}

void task3()
{
  while (thread[2].runningT)
  {
    terminal_writestring("Running thread: <3>");
    Enter();
    for (float i = 0; i < 100000; i +=0.01){};
    thread[2].runningT--;
    yield();
  }
}

void initTIDq()
{
  int i;
  for (i = 0; i < N; i++)
    tidq[i].TID = i;
}

int get_TID()
{
  int i;
  for (i = 0; i < N; i++)
  {
    if (!tidq[i].used)
    { 
      tidq[i].used = 1;
      return i;
    }
  }
  return -1;
}


void thread_create(void *stack, void *task)
{
  uint32_t TID = get_TID();
  if (TID == -1)
    return;

    char *p = "";
    itoa(p,'d',TID);
    terminal_writestring(" ");
    terminal_writestring("Create number: ");
    terminal_writestring(p);
    terminal_writestring(" thread!");
    Enter();

  thread[TID].task = task;
  thread[TID].TID = TID;
  thread[TID].bp = (uint32_t *)stack;
  thread[TID].flag = 0;
  thread[TID].status = 1;
  thread[TID].finished = 0;
  thread[TID].comeT = TID;
  thread[TID].runningT = 0;

  if (FIFO)
  {
    thread[TID].priority = &(thread[TID].comeT);
  }

  else if (SRT)
  {
    thread[TID].priority = &(thread[TID].runningT);
  }

  thread[TID].sp = (uint32_t *)stack - 11;
  //(uint32_t *)(((uint16_t *)stack)-23);

  *((uint32_t *)stack -  0) = (uint32_t)task;   // EIP   0 1
  *((uint32_t *)stack -  1) = 0;                // FLAGS 2 3
  *((uint32_t *)stack -  2) = 0;                // EAX   4 5 
  *((uint32_t *)stack -  3) = 0;                // ECX   6 7
  *((uint32_t *)stack -  4) = 0;                // EDX   8 9 
  *((uint32_t *)stack -  5) = 0;                // EBX   10 11
  *((uint32_t *)stack -  6) = 0;                // TEMP ESP  12 13
  *((uint32_t *)stack -  7) = 0;                // EBP       14 15
  *((uint32_t *)stack -  8) = 0;                // ESI       16 17
  *((uint32_t *)stack -  9) = 0;                // EDI       18 19

  // Because segment register just have 16 bits 
  // so when we push and pop them the pointer just move
  // 16 bits. For this reason, we should count our stack as 16 bits
  // per entry.
  *(((uint16_t *)stack) -  20) = 0x10;    // DS
  *(((uint16_t *)stack) -  21) = 0x10;    // ES
  *(((uint16_t *)stack) -  22) = 0x10;    // FS
  *(((uint16_t *)stack) -  23) = 0x10;    // GS

  // initial the heap
  readyHeap[TID] = &thread[TID];
  heapLenght++;
}


void initSys()
{
  initTIDq();
  thread_create(&stack1[1023], task1);
  thread[0].runningT = 5;
  thread_create(&stack2[1023], task2);
  thread[1].runningT = 3;
  thread_create(&stack3[1023], task3);
  thread[2].runningT = 4;
  initHeap();
}

void init( multiboot* pmb ) {

  terminal_initialize();
  terminal_writestring("Welcome FIFO-1 System");
  Enter();

  initSys();

  schedule();
  return;
}

