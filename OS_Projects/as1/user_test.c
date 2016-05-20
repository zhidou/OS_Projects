#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define KEYBOARD_TEST _IOR(0, 6, struct keyborad_data_structure)

int main () {

  /* attribute structures */
  struct keyborad_data_structure
  {
    int shift;
    char character;
  }keyborad_data;

  int fd = open ("/proc/ioctl_keyboard_test", O_RDONLY);

  while (1)
  {
   ioctl(fd, KEYBOARD_TEST, &keyborad_data);
   putchar(keyborad_data.character);
   putchar('\n');
   printf("We already output a char\n");
  }
  
  return 0;
}