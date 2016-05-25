// choose the unique major num for the driver
// you could check the used num in /usr/include/linux/major.h
# define major_num 500
		
typedef struct 
{
	char *filname;		// file name
	int fdt_index;		// file descriptor table index
	int	position;			// the offset in its block
	char *input;		// input content
	int size;			// input size
}file_info_t;

// define ioctl request
# define IOCTL_PWD _IOR(major_num, 0, char *)
# define IOCTL_LS _IOR(major_num, 1,char **)
# define IOCTL_CREAT _IOWR(major_num, 2, char *)
# define IOCTL_MKDIR _IOWR(major_num, 3, char *)
# define IOCTL_UNLINK _IOWR(major_num, 4, char *)
# define IOCTL_WRITE _IOWR(major_num, 5, file_info_t*)
# define IOCTL_READ _IOWR(major_num, 6, file_info_t*)
# define IOCTL_READDIR _IOWR(major_num, 7, file_info_t*)
# define IOCTL_OPEN _IOWR(major_num, 8, char *)
# define IOCTL_CLOSE _IOWR(major_num, 9, int*)
# define IOCTL_LSEEK _IOWR(major_num, 10, file_info_t*)
