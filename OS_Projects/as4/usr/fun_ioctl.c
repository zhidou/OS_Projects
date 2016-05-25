# include "ramdisk.h"

static int fd = -1;
static int counter = 0;
int openProc(void)
{
	if (fd != ERROR)
		return FINE;

	fd = open("/proc/Ramdisk", O_RDWR);
	counter++;
	if (fd == ERROR)
	{
		printf("Failed to open Ramdisk!\n");
		return ERROR;
	}
	return FINE;
}

void rd_pwd(void)
{
	if (openProc() == ERROR)
		return;
	char cur_path[100];
	ioctl(fd, IOCTL_PWD, cur_path);
	printf("%s\n", cur_path);
}

void rd_ls(void)
{
	if (openProc() == ERROR)
		return;
	char cur_path[1024][14];
	if (ioctl(fd, IOCTL_LS, cur_path) == ERROR)
		return;
	int i, j = 1;
	for (i = 0; i < 1024; i++)
	{
		if (cur_path[i][0] != 0)
		{
			if (j % 4 == 0 && j != 0)
				printf("\n");
			printf("%s        ", cur_path[i]);
			j++;
		}
	}
	printf("\n");
}


int check_pathname(char *pathname)
{	// just check the first character of filename
	// if there is extra space in the delete
	// if begin with non digit, letter, illegal
	char *p;

	for (p = pathname; *p == ' '; p++);
	if (*p == '\0')
		return ERROR;
	else
	{	
		if ((*p < 46 || *p >57) && (*p <65 || *p > 90) && (*p < 97 || *p > 122))
			return ERROR;
		if (*p == '.')
		{
			if (*(p+1) != '.' && *(p+1) != '/')
				return ERROR;
			if (*(p+1) == '.' && *(p+2) != '/')
				return ERROR;
		}
		if (p != pathname)
			strcpy(pathname, p);
	}

	for (p = pathname + strlen(pathname); p > pathname; p--)
	{	
		if (*p == ' ')
			*p = '\0';
		if ((*p > 45 && *p < 58) || (*p > 64 && *p < 91) || (*p > 96 && *p < 123)) 
			break;
	}

	return FINE;
}


int rd_creat(char *pathname)
{
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if ((ret = ioctl(fd, IOCTL_CREAT, pathname)) == ERROR)
		return ERROR;
	return ret;
}

int rd_mkdir(char *pathname)
{
	if (openProc() == ERROR)
		return ERROR;
	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if (ioctl(fd, IOCTL_MKDIR, pathname) == ERROR)
		return ERROR;
	else return FINE;
}

int rd_open(char *pathname)
{
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if ((ret=ioctl(fd, IOCTL_OPEN, pathname)) == ERROR)
		return ERROR;
	return ret;
}

int rd_unlink(char *pathname)
{
	if (openProc() == ERROR)
		return ERROR;
	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if (ioctl(fd, IOCTL_UNLINK, pathname) == ERROR)
		return ERROR;
	else return FINE;
}

int rd_write(int fdd, char *input, int num_bytes)
{
	file_info_t file_info;

	file_info.fdt_index = fdd;
	file_info.size = num_bytes;
	file_info.input = input;
	printf("writting bytes %d\n", file_info.size);	
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	
	ret = ioctl(fd, IOCTL_WRITE, &file_info);
	printf("%d words has been writen\n", ret);
	return ret;
}

int rd_read(int fdd, char *output, int num_bytes)
{
	file_info_t file_info;
	file_info.fdt_index = fdd;
	file_info.size = num_bytes;
	file_info.input = output;
	printf("reading bytes %d\n", file_info.size);	
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	ret = ioctl(fd, IOCTL_READ, &file_info);
	printf("%d words has been read\n", ret);
	printf("%s\n", file_info.input);
	return ret;
}

int rd_readdir(int fdd, char *address)
{
	file_info_t file_info;
	file_info.fdt_index = fdd;
	file_info.input = address;
		
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	ret = ioctl(fd, IOCTL_READDIR, &file_info);
	int *a = (int*)&file_info.input[14];
	printf("%d\n", *a);
	return ret;
}


int rd_close(int fdd)
{
	int ret;
	if (openProc() == ERROR)
		return ERROR;
	if ((ret=ioctl(fd, IOCTL_CLOSE, &fdd)) == ERROR)
		return ERROR;
	return ret;
}

 int rd_lseek(int position, int fdd)
 {
 	file_info_t file_info;
	file_info.position = position;
	file_info.fdt_index = fdd;
	int ret;
	ret = ioctl(fd, IOCTL_LSEEK, &file_info);
	return ret;
 }




















