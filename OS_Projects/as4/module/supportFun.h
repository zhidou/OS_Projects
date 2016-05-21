//
// Created by DouZhi on 16/5/18.
//

# ifndef _supportFun_
# define _supportFun_
# include "init_module.h"

typedef struct file_object_T
{
    int inode_num;
    int file_position;
    struct file_object_T *next;
}file_object_t;

typedef struct FDT_T
{
    int pid;
    file_object_t *file_object;
    char *cur_path;
    struct FDT_T *next;
}FDT_t;

FDT_t * create_FDT(void);
FDT_t * has_FDT(void);

// return the right dir to do the operation
int partition(char *pathname, FDT_t *FDT, char *parentfile[20], char ** childfile);
int is_exist(int inode_index, char *filename, int *position);
int fill_file(int des_index, int *position, char *content, int size, char mode);
int get_list(int des_index, char list[][14]);
int alloc_inode(int parent_inode_index, char type[4]);
int vfree_inode(char *filename, int des_index);
int is_open(int des_index, FDT_t *FDT);
char * get_blockpointer(int position, inode_t *des_inode);
# endif
