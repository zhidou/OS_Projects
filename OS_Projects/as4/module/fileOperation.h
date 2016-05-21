//
// Created by DouZhi on 16/5/18.
//

# ifndef _fileOperation_
# define _fileOperation_
# include "supportFun.h"

int create(char *pathname, char *type);
int rd_creat(char *pathname);
int rd_mkdir(char *pathname);
int rd_unlink(char *pathname);
int rd_open(char *pathname);
int rd_close(int fd);
int rd_write(int fd, char *input, int num_bytes);
int rd_read(int fd, char *output, int num_bytes);
int rd_readdir(int fd, char *address);
int rd_lseek(int position, int fd);


char* rd_pwd(void);
int rd_ls(char list[][14]);
# endif