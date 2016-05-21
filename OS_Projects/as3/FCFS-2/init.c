#include "multiboot.h"
#include "types.h"
#include "fun.h"


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
int curThreadID = -1;
int indexi = -1;
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

uint32_t curFlag;


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

// we use a heap to maintain these ready thread
// for heap is more extendable. 
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
  // to check if there is any thread active
  if (!heapLenght)
  {
    terminal_writestring("No more ready thread!");
    __asm__ volatile("jmp loop\n\t");
  }

  // if there is, get reset the came time of current thread
  // basically this if for FIFO, for re-sort the head
  curThread->comeT = getTick();

  TCB *next = next_thread();

  TCB *temp = curThread;

  curThread = next;
  
  __asm__ volatile("call switch_to\n\t"::"S"(temp), "D"(next));

}


void ThreadFinish()
{
  terminal_writestring("Current Thread: ");
  printID(curThread->TID + 1);
  terminal_writestring(" is done!!");
  Enter();
  // set current thread finished flag
  curThread->finished = 1;
  // minus one from heap length, basically this means this thread 
  // leaves ready heap.
  heapLenght--;

  // recycle availible ID
  tidq[curThread->TID].used = 0;
  // for the good of demo, we disable interrup, when running the thread function
  // now enable interrupt, thus we could change the context to another thread
  _enable_interrupt();
} 

void task1()
{
  while ((curThread->runningT)--)
  {
    // for good effect, we disable interrupt when we execute 
    // the function of current thread, so that we could see
    // it print <ID> one by one, but not just show the result
    // But in real life, we do not do in that way
    _disable_interrupt();

    // the fake work. we should keep this function running 
    // certain time, so that, the interrup could come. For that
    // reason we do a loop here, to pretend the thread is working
    for (float i = 0; i < 1000000; i+= 0.1){};

    terminal_writestring("Running Thread <1>");
    Enter();

    // if the thread all done its work, then it should call finish
    if (!(curThread->runningT))
      ThreadFinish();

    _enable_interrupt();
    // Because we do lots of "job", during the interrupt disable time
    // however, during this time, interrupt actually happens, or we could
    // say, the PIC receive the interrupt requirement, but our CPU do not care
    // about it, because we disable interrupt. Thus when we enable interrupt
    // CPU will check whether there is any body want interrupt service. 
    // certainly there is. Thus we will switch contest.
  }
}

void task2()
{
  while ((curThread->runningT)--)
  {
    _disable_interrupt();
    for (float i = 0; i < 1000000; i+= 0.1){};
    terminal_writestring("Running Thread <2>");
    Enter();
    if (!(curThread->runningT))
      ThreadFinish();
    _enable_interrupt();
  }
}

void task3()
{
  while ((curThread->runningT)--)
  {
    _disable_interrupt();
    for (float i = 0; i < 1000000; i+= 0.1){};
    terminal_writestring("Running Thread <3>");
    Enter();
    if (!(curThread->runningT))
      ThreadFinish();
    _enable_interrupt();
  }
}

// initialize ID pool
void initTIDq()
{
  int i;
  for (i = 0; i < N; i++)
    tidq[i].TID = i;
}

// we use a ID pool to control the number of thread
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

  
  char p[10];
  itoa(p,'d',TID);
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
    thread[TID].priority = &(thread[TID].comeT);

  else if (SRT)
    thread[TID].priority = &(thread[TID].runningT);

  thread[TID].sp = (uint32_t *)stack - 11;
  
  // we should make a illusion that it seems this thread has been 
  // executed before, when actually this is the first time
  // But actually this is great, because we could make the thread do 
  // want we want it to do, we could set the register, the next instrcution
  // it will go. How great! but one thing is you shoud take care of the order
  *((uint32_t *)stack - 0) = (uint32_t)task;   // EIP
  
  // we set this means, enable interrupt. Because when we execute the thread 
  // for the first time, this thread jump from switch directly to its function
  // that means we are skip iret, witch means we should set the EFLAGS by ourselves
  *((uint32_t *)stack - 1) = 0x202;   // FLAGS
  
  // pushal
  *((uint32_t *)stack - 2) = 0;                // EAX
  *((uint32_t *)stack - 3) = 0;                // ECX
  *((uint32_t *)stack - 4) = 0;                // EDX
  *((uint32_t *)stack - 5) = 0;                // EBX
  *((uint32_t *)stack - 6) = 0;                // TEMP ESP
  *((uint32_t *)stack - 7) = 0;                // EBP
  *((uint32_t *)stack - 8) = 0;                // ESI
  *((uint32_t *)stack - 9) = 0;                // EDI

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

  _disable_interrupt();
  terminal_initialize();
  terminal_writestring("Welcome FIFO-2 System");
  Enter();
  initSys();
  initIDT();
  init_PIC();
  init_timer(50);
  _enable_interrupt();

  while(1){};
  return;
}

