obj-m += ramdisk_module.o
ramdisk_module-objs := init_module.o fileOperation.o supportFun.o
all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	
