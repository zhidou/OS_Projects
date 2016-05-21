//
// Created by DouZhi on 16/5/18.
//

# include "fileOperation.h"

extern superblock_t *superblock;
extern inode_t *inode;

// we don't allow same name file if even they are different type
int create(char *pathname, char *type)
{
    int des_inode_index;
    FDT_t *FDT;
    char *parentfile[20];
    char *childfile;
    int i; 		// for any loop
    dirctory_entry_t *dirctory_entry;
    char para[16];

    int temp = -1;
    int *position = &temp;

    if (superblock->vfreeBlock < 1 || superblock->vfreeInode < 1)
    {
        printk("Insufficient Memory!\n");
        return ERROR;
    }


    FDT = has_FDT();

    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;


    des_inode_index = partition(pathname, FDT, parentfile, &childfile);

    // check correctness of parentfile path and childfile
    // check whether parentfile exist in coresponding path. it must exist
    // otherwise it reture ERROR

    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = is_exist(des_inode_index, parentfile[i], position)) == ERROR)
            return ERROR;

    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;

    // if exist the same name file
    if (is_exist(des_inode_index, childfile, position) != ERROR)
        return ERROR;

    // write into cur_path

    dirctory_entry = (dirctory_entry_t *)vmalloc(sizeof(uint8_t) * 16);

    strcpy(dirctory_entry->filename, childfile);
    dirctory_entry->inode_index = superblock->first_vfree_inode;

    if (!strcmp(type , reg))
    {
        int len = strlen(childfile);
        char suffix[5] = ".txt";
        if (len + 5 <= 14)
            strcpy(dirctory_entry->filename + len, suffix);
        else
            strcpy(dirctory_entry->filename + 9, suffix);
    }

    for (i = 0; i < 16; i++)
        *(para + i) = *((char *)dirctory_entry + i);


    if (fill_file(des_inode_index, &(FDT->file_object->file_position), para, 16, 'c') == ERROR)
    {
        vfree(dirctory_entry);
        return ERROR;
    }

    if (!strcmp(type , dir))
        alloc_inode(des_inode_index, dir);
    else
        alloc_inode(des_inode_index, reg);

    vfree(dirctory_entry);
    return FINE;

}


int rd_creat(char *pathname)
{
    return create(pathname, reg);
}

int rd_mkdir(char *pathname)
{
    return create(pathname, dir);
}

int rd_unlink(char *pathname)
{
    int des_inode_index;
    int delet_inode_index;
    FDT_t *FDT;
    char *parentfile[20];
    char *childfile;
    int i; 		// for any loop
    inode_t *des_inode;
    int temp = -1;
    int *position = &temp;

    // (4) if you attempt to unlink the root directory file.
    if (!strcmp(pathname, "/"))
        return ERROR;

    FDT = has_FDT();

    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;

    des_inode_index = partition(pathname, FDT, parentfile, &childfile);

    // (1) if the pathname does not exist
    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = is_exist(des_inode_index, parentfile[i],position)) == ERROR)
            return ERROR;

    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;
    // check whether the file need to delet exist
    if ((delet_inode_index = is_exist(des_inode_index, childfile, position)) == ERROR)
        return ERROR;

    des_inode = &inode[delet_inode_index];

    // (2) if you attempt to unlink a non-empty directory file,
    if (!strcmp(des_inode->type, dir) && des_inode->size > 0)
        return ERROR;

    // (3) if you attempt to unlink an open file
    if (is_open(delet_inode_index, FDT) == ERROR)
        return ERROR;

    vfree_inode(childfile, delet_inode_index); // change the num of vfree inode and first available

    return FINE;
}


int rd_open(char *pathname)
{
    int des_inode_index;
    int open_inode_index;
    FDT_t *FDT;
    char *parentfile[20];
    char *childfile;
    int i; 		// for any loop
    inode_t *des_inode;
    int temp = -1;
    int *position = &temp;
    int fd = 0;
    char temppath[1024];
    char *temppathp = temppath;
    char backup[1024];
    int b = 0;
    FDT = has_FDT();
    strcpy(backup, pathname);

    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;

    des_inode_index = partition(pathname, FDT, parentfile, &childfile);

    // (1) if the pathname does not exist
    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = is_exist(des_inode_index, parentfile[i],position)) == ERROR)
            return ERROR;

    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;

    // check whether the file need to open exist
    if ((open_inode_index = is_exist(des_inode_index, childfile, position)) == ERROR)
        return ERROR;

    des_inode = &inode[open_inode_index];
    // (3) if you attempt to open an open file
    if (is_open(open_inode_index, FDT) == ERROR)
        return ERROR;

    // load into FDT
    file_object_t *file_object = FDT->file_object;
    if (!strcmp(des_inode->type, dir))
        fd = 0;
    else
    {
        while (file_object->next != NULL){
            file_object = file_object->next;
            fd++;
        }
        file_object->next = (file_object_t *)vmalloc(sizeof(file_object_t));
        fd++;
        file_object = file_object->next;
    }

    file_object->inode_num = open_inode_index;
    file_object->file_position = 0;
    file_object->next = NULL;


    // build new path
    strcpy(temppathp, FDT->cur_path);
    temppathp += strlen(FDT->cur_path);
    if (strcmp(FDT->cur_path, "/"))
    {  
        *temppathp = '/';
        temppathp++;
        *temppathp = '\0';
    }

    int back = 0;
    while (strncasecmp(&backup[b], "../", 3) == FINE)
    {
        back++;
        b += 3;
    }

    while (back > 0)
    {
        for (temppathp -= 2; *temppathp != '/'; temppathp--);
        back--;
        temppathp++;
    }

    if (strncasecmp(backup, "./", 2) == FINE)
        b += 2;

    if (strncasecmp(backup, "/", 1) == FINE)
    {
        b ++;
        temppathp = &temppath[1];
    }
    strcpy(temppathp, &backup[b]);
    vfree(FDT->cur_path);
    FDT->cur_path = (char *)vmalloc(strlen(temppath) + 1);
    strcpy(FDT->cur_path, temppath);

    return fd;
}


int rd_close(int fd)
{
    inode_t *close_inode;
    FDT_t *FDT;
    int i;
    FDT = has_FDT();
    file_object_t *file_object = FDT->file_object;
    if (fd == 0)
    {
        if (file_object->inode_num == 0)
            return ERROR;
        close_inode = &inode[file_object->inode_num];
        file_object->inode_num = close_inode->parent_index;
        file_object->file_position = 0;
        for (i = strlen(FDT->cur_path); i > 0; i--)
            if (FDT->cur_path[i] == '/')
                break;
        FDT->cur_path[i + 1] = '\0';
        return FINE;
    }
    file_object_t *fomer;
    for (i = 0; i < fd; i++)
    {
        if (file_object == NULL)
            return ERROR;
        fomer = file_object;
        file_object = file_object->next;
    }
    fomer->next = file_object->next;
    vfree(file_object);
    return FINE;
}

int rd_write(int fd, char *input, int num_bytes)
{
    FDT_t *FDT;
    int i;
    FDT = has_FDT();
    file_object_t *file_object = FDT->file_object;
    int ret;

    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object == NULL)
            return ERROR;
        file_object = file_object->next;
    }
    if (file_object == NULL)
        return ERROR;
    ret = fill_file(file_object->inode_num, &(file_object->file_position), input, num_bytes, 'w');

    return ret;
}

int rd_read(int fd, char *output, int num_bytes)
{
    FDT_t *FDT;
    FDT = has_FDT();
    file_object_t *file_object = FDT->file_object;
    inode_t *des_inode;
    char *readPointer;
    int i = 0;
    int readBytes;
    char *tempoutput = output;
    int position;

    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object == NULL)
            return ERROR;
        file_object = file_object->next;
    }
    if (file_object == NULL)
        return ERROR;

    des_inode = &inode[file_object->inode_num];

    if (des_inode->size == 0)
        return ERROR;
    if (des_inode->location[0] == NULL)
        return ERROR;

    position = file_object->file_position >= des_inode->size ? 0 : file_object->file_position;

    readBytes = (des_inode->size - position) < num_bytes ? (des_inode->size - position) : num_bytes;
    num_bytes = readBytes;
    *(output + num_bytes) = '\0';
    
    while (readBytes > 0)
    {
        readPointer = get_blockpointer(position, des_inode);
        if (readPointer == ERROR)
            return ERROR;
        int remain = block_size - position % block_size;
        int iterationRead = readBytes < remain ? readBytes : remain;

        memcpy(tempoutput, readPointer, iterationRead);
        tempoutput += iterationRead;
        readBytes -= iterationRead;
        position += remain;
    }

    return num_bytes;
}


int rd_readdir(int fd, char *address)
{
    FDT_t *FDT;
    FDT = has_FDT();
    file_object_t *file_object = FDT->file_object;
    int size;
    inode_t *des_inode;
    int position;
    if (fd != 0)
        return ERROR;

    if (file_object == NULL)
        return ERROR;
    des_inode = &inode[file_object->inode_num];

    size = des_inode->size / sizeof(dirctory_entry_t);

    position = file_object->file_position;

    if (position >= size)
    {
        file_object->file_position %= size;
        return 0;
    }

    if (size == 0)
        return ERROR;
    if (des_inode->location[0] == NULL)
        return ERROR;
    int blockIndex;
    int inside_offside = position % 16;
    dirctory_entry_t *dirctory_entry;
    int len = sizeof(dirctory_entry_t);
    if (position < 128)
    {
        blockIndex = position / 16;
        dirctory_entry = (dirctory_entry_t *)des_inode->location[blockIndex] + inside_offside;
        memmove(address, dirctory_entry, len);
    }
    else if (position - 128 < 1024)
    {
        uint32_t **single;
        blockIndex = (position - 128) / 16;
        if (des_inode->location[8] == NULL)
            return ERROR;
        single = (uint32_t **)des_inode->location[8];
        dirctory_entry = (dirctory_entry_t *)single[blockIndex] + inside_offside;
        memmove(address, dirctory_entry, len);
    }
    else
    {
        int x = (position - 128 - 1024) / 64;
        int y = (position - 128 - 1024) % 64;
        uint32_t ***double_p;
        blockIndex = position / 16;

        double_p= (uint32_t ***)des_inode->location[9];
        dirctory_entry = (dirctory_entry_t *)double_p[x][y];
        memmove(address, dirctory_entry, len);
    }

    file_object->file_position++;


    return file_object->file_position > 0;
}

int rd_lseek(int position, int fd)
{
    FDT_t *FDT;
    FDT = has_FDT();
    inode_t *des_inode;
    file_object_t *file_object = FDT->file_object;
    int size;
    int i;

    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object == NULL)
            return ERROR;
        file_object = file_object->next;
    }
    if (file_object == NULL)
        return ERROR;

    des_inode = &inode[file_object->inode_num];
    size = des_inode->size;

    if (size < position)
        position = size;

    file_object->file_position = position;
    return FINE;
}

char * rd_pwd(void)
{
    FDT_t *FDT;
    FDT = has_FDT();
    return FDT->cur_path;
}

int rd_ls(char list[][14])
{
    FDT_t *FDT;
    FDT = has_FDT();
    int inode_index = FDT->file_object->inode_num;

    if (get_list(inode_index, list) == ERROR)
        return ERROR;
    return FINE;
}