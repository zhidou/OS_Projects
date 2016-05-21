void terminal_initialize();
void Enter();
void terminal_writestring(const char* data);
void itoa (char *buf, int base, uint32_t d);
void printID(uint16_t ID);

uint32_t getTick();
void initIDT();
void init_PIC();
void init_timer(uint16_t x);