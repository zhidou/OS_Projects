#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");

struct keyborad_data_structure
{
	int shift;
	char character;
};// That is for the dev_id

static struct keyborad_data_structure keyborad_data;


struct keyboard_status_structure
{
	int ready;
};// This is for the status of the keyboard

static struct keyboard_status_structure keyboard_status;


DECLARE_WAIT_QUEUE_HEAD(keyboard_wait_queue);

#define KEYBOARD_TEST _IOR(0, 6, struct keyborad_data_structure)
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static inline unsigned char inb( unsigned short usPort ) {
  unsigned char uch; 
  asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
  return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {
  asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
} 

void my_getchar ( void ) {
	

  char c;

  static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

  // To my understanding, or as far as i know, how many key we press
  // correspondingly we will have that many interrupt
  // and when we press the key, the data register is continuiously be filled
  // with the data of this key. 
  // And actually throught observe, I found both press and release will cause interrupt, but just when press
  // we will output the character, so the interrupt caused by release, we just do nothing. Except, when we release 
  // the shitf, the flag of shift should change.

  if ((c = inb(0x60)) & 0x80) // when the 7th bit is 1, means release, 0, means press
  {
  	c &= 0x7f;                 // get the 7 bits 
  	if (c == 0x2a || c == 0x36) // if it is shift...
  		keyborad_data.shift = 0;
  	return; // this return for realse. we finished the request
  }

  if (c == 0x2a || c == 0x36)
  {
  	keyborad_data.shift = 1;
  	return;
  } // this return for shift, that means a shift interrupt!

  // know except the case of shift, we could get the character we want!
  keyborad_data.character = scancode[(int)c];
  
  keyboard_status.ready = 1; // now we get want we want!!!
  
  if (keyborad_data.shift == 1)
  {
  	if (keyborad_data.character >= 0x61 && keyborad_data.character <=0x7A)
  		keyborad_data.character -= 0x20;
  }

}

irqreturn_t keyboard_handler(int irq, void * dev_id)
{
	printk("<1> we are in inturrupt\n");
	my_getchar();
	if (keyboard_status.ready == 1)
    // give a semaphore, if ready, then wake up the process
		wake_up_interruptible(&keyboard_wait_queue);
	return IRQ_HANDLED;
}

static int __init initialization_routine(void) 
{
  printk("<1> Loading module\n");
  request_irq(1,keyboard_handler ,IRQF_SHARED,"keyboard_module_test", (void *) &keyborad_data);
  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
  proc_entry = create_proc_entry("ioctl_keyboard_test", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

	proc_entry->proc_fops = &pseudo_dev_proc_operations;

	return 0;
}

static void __exit cleanup_routine(void) 
{
  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_keyboard_test", NULL);
  free_irq(1,(void *)&keyborad_data); 
  return;
}


static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
  unsigned long copied;
  switch (cmd){

  case KEYBOARD_TEST:
    // to check whether the keyboard is ready to output.
    wait_event_interruptible(keyboard_wait_queue, (keyboard_status.ready == 1));
    // reset the ready value.
    keyboard_status.ready = 0;
    copied = copy_to_user((struct k *)arg, &keyborad_data,sizeof(keyborad_data));
    break;
  
  default:
    return -EINVAL;
    break;
  }
  
  return 0;
}

module_init(initialization_routine);
module_exit(cleanup_routine);

