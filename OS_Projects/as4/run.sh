cd module
if [ -a ramdisk_module.ko ]; then
	make clean
fi
make
insmod ramdisk_module.ko
cd ../usr
if [ -x ramdisk ]; then
	make clean
fi
make
./ramdisk
