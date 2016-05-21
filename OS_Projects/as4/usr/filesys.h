# ifndef _TEST_
# define _TEST_

# include <stdio.h>
# include <fcntl.h>
# include <string.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <dirent.h>
# include "filesys_ioctl.h"





# define FINE 0
# define ERROR -1
# define reg "reg"
# define dir "dir"
# define testdata data3
#define MAX_FILES 524
#define BLK_SZ 256		/* Block size */
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */
#define PATH_PREFIX ""




int openProc(void);
void rd_pwd(void);
void rd_ls(void);
int rd_creat(char *pathname);
int rd_mkdir(char *pathname);
int rd_unlink(char *pathname);
int rd_write(int fdt_index, char *content, int size); 
int rd_open(char *pathname);
int rd_close(int fd);
int rd_write(int fd, char *input, int num_bytes);
int rd_read(int fd, char *output, int num_bytes);
int rd_readdir(int fd, char *address);
int rd_lseek(int position, int fd);
# endif