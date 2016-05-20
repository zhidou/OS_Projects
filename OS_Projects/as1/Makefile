obj-m += keyboard_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc user_test.c -o user_test
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user_test
