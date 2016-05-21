//
// Created by DouZhi on 16/5/17.
//

# include "supportFun.h"

FDT_t *FDT_h = NULL;
extern inode_t *inode;
extern void *space;
extern bitmap_t *bitmap;
extern superblock_t *superblock;
extern void *space;

FDT_t * create_FDT(void)
{
    FDT_t *FDT;
    if ((FDT = (FDT_t *)vmalloc(sizeof(FDT_t))) == NULL)
        return NULL;

    FDT->pid = current->pid;

    FDT->file_object = (file_object_t *)vmalloc(sizeof(file_object_t));
    FDT->file_object->inode_num = 0;
    FDT->file_object->file_position = 0;
    FDT->file_object->next = NULL;

    FDT->cur_path = (char *)vmalloc(3);
    strcpy(FDT->cur_path, "/");
    FDT->next = NULL;

    return FDT;
}

FDT_t * has_FDT()
{

    if (FDT_h == NULL)
    {
        if ((FDT_h = create_FDT()) != NULL)
        {
            return FDT_h;
        }
        else return ERROR;
    }
    int pid = current->pid;
    int ppid = current->parent->pid;
    FDT_t *FDT = FDT_h;
    FDT_t *FDT_p;

    while (FDT->next != NULL && FDT->pid != pid)
    {
        if (FDT->pid == ppid)
            FDT_p = FDT;
        FDT = FDT->next;
    }

    if (FDT->pid == pid)
        return FDT;

    FDT_p = FDT;

    if (FDT_p->pid == ppid)
    {
        FDT->next = (FDT_t *)vmalloc(sizeof(FDT_t));
        FDT = FDT->next;
        FDT->pid = pid;

        FDT->file_object = (file_object_t *)vmalloc(sizeof(file_object_t));
        file_object_t *file_object_p = FDT_p->file_object;
        file_object_t *file_object_c = FDT->file_object;

        while (file_object_p->next != NULL)
        {
            *(file_object_c) = *(file_object_p);
            file_object_c->next = (file_object_t *)vmalloc(sizeof(file_object_t));
            file_object_p = file_object_p->next;
            file_object_c = file_object_c->next;
        }

        *(file_object_c) = *(file_object_p);
        file_object_c->next = NULL;

        FDT->cur_path = (char *)vmalloc(strlen(FDT_p->cur_path) + 1);
        strcpy(FDT->cur_path, FDT_p->cur_path);

        FDT->next = NULL;
    }
    else
    {
        FDT->next = create_FDT();
        FDT = FDT->next;
    }
    return FDT;
}

int destroy_FDT(void)
{
    FDT_t *temp;
    while (FDT_h != NULL)
    {
        temp = FDT_h->next;

        file_object_t *temp_f;
        while (FDT_h->file_object != NULL)
        {
            temp_f = FDT_h->file_object->next;
            vfree(FDT_h->file_object);
            FDT_h->file_object = temp_f;
        }

        vfree(FDT_h->cur_path);

        vfree(FDT_h);
        FDT_h = temp;
    }
    return FINE;
}

int layer_check(char *pathname)
{
    char *p;
    int counter = 0;
    for (p = pathname; *p != '\0'; p++)
        if (*p == '/')
            counter++;
    return counter;
}

int partition(char *pathname, FDT_t *FDT, char *parentfile[20], char **childfile)
{
    char *pn = pathname;
    int back = 0;
    inode_t *des_inode;		// the destination inode
    int des_index;			// the index of destination inode
    des_index = FDT->file_object->inode_num;
    des_inode = &inode[des_index];

    // adjust the destination inode

    while (strncasecmp(pn, "../", 3) == FINE)
    {
        back++;
        pn +=3;
    }

    if (back > 0 && back > layer_check(FDT->cur_path))
        return ERROR;

    while (back > 0)
    {
        des_index = des_inode->parent_index;
        des_inode = &inode[des_index];
        back--;
    }

    if (strncasecmp(pn, "./", 2) == FINE)
        pn += 2;

    if (strncasecmp(pn, "/", 1) == FINE)
    {
        pn ++;
        des_index = 0;
    }

    // partition, home/share/as4->home\0share\0as4\0
    int length;
    length = strlen(pn);
    int i, j, k;
    k = 0;
    for (i = 0, j = 0; i < length; i++)
    {
        if (pn[i] != '/')
            j++;
        else
        {
            parentfile[k] = &pn[i - j];
            parentfile[k][j] = '\0';
            j = 0;
            k++;
        }

    }

    *childfile = &pn[i - j];

    return des_index;
}

int search(uint32_t *location[], int size, int *times, char *filename, int *position)
{
    int i, j;		// for loop

    dirctory_entry_t *dirctory_entry;

    // you should search all the location....

    // search the first 8 directory location
    for (i = 0; i < size && *times > 0; i ++)
    {
        dirctory_entry = (dirctory_entry_t *)location[i];
        if (dirctory_entry == NULL)
            return ERROR;
        // one block is 256 bytes, and one dirctory entry is 16 bytes
        // thus one block could at most save 16 entry
        for (j = 0; j < 16 && *times > 0; j ++)
        {
            (*position)++;
            if ((dirctory_entry + j)->filename == NULL)
                return ERROR;
            if(!strcmp((dirctory_entry + j)->filename, filename))
                return (dirctory_entry + j)->inode_index;
            (*times)--;
        }
    }

    return FINE;
}




// search whether file exist in current path, if exist
// return inode_index, otherwise return ERROR.
int is_exist(int inode_index, char *filename, int *position)
{
    int ret;

    if (strlen(filename) > 13)
        return ERROR;

    inode_t *des_inode;
    des_inode = &inode[inode_index];
    uint32_t size = des_inode->size;
    uint32_t temp = size/16;
    int *times = &temp;

    if (*times <=0 )
        return ERROR;


    // search direct location
    if ((ret = search((uint32_t **)des_inode->location, 8, times, filename, position)) != FINE)
        return ret;
    if (*times <=0 )
        return ERROR;

    // search the single_indirect location
    uint32_t **single_indirect;

    if ((single_indirect = (uint32_t **)(des_inode->location[8])) == NULL)
        return ERROR;

    if ((ret = search(single_indirect, 64, times, filename, position)) != FINE)
        return ret;

    if (*times <=0 )
        return ERROR;
    // search the double_indirect location
    uint32_t ***double_indirect;
    if ((double_indirect = (uint32_t ***)des_inode->location[9]) == NULL)
        return ERROR;
    // a double_indirect pointer point to a table, contains 64 single_indirect pointer...
    int i;
    for(i = 0; i < 64; i++)
    {
        single_indirect = double_indirect[i];
        if (single_indirect == NULL)
            return ERROR;
        if ((ret = search(single_indirect, 64, times, filename, position)) != FINE)
            return ret;
        if (*times <=0 )
            return ERROR;
    }

    return ERROR;
}

int alloc_bitmap(int bit_index)
{
    int unit_index = bit_index / 8;
    int offset = bit_index % 8;
    uint8_t temp = 0x80;
    temp >>= offset;
    bitmap->index[unit_index] |= temp;

    // get the next vfree bit
    int i;
    int j;
    for (i = unit_index, j = 8; i < bitmap_sublock && j == 8; i++)
        for (j = 0; j < 8; j++)
            if (!(bitmap->index[i] & (0x80 >> j)))
                break;
    i --;
    bit_index = i * 8 + j;
    if (bit_index > allocatable_space_size)
        bit_index = ERROR;

    superblock->vfreeBlock--;
    superblock->first_vfree_bitmap_block = (uint16_t) bit_index;

    return FINE;
}


char * alloc_block(inode_t *des_inode)
{
    int location_index = des_inode->offset[0] + 1;

    // this file reach it max size
    if (location_index > 4167)
        return ERROR;

    if (superblock->vfreeBlock < 1)
        return ERROR;

    // need single_indirect pointer, which at least need 2 block
    if (location_index == 8 && superblock->vfreeBlock < 2)
        return ERROR;

    // need double_indirect pointer, which at least need 3 block
    if (location_index == 72 && superblock->vfreeBlock < 3)
        return ERROR;
    //int location_index = des_inode->offset[0] + 1;
    int block_index = superblock->first_vfree_bitmap_block;
    alloc_bitmap(block_index);

    char * pointer;
    if (location_index < 8)
    {
        des_inode->location[location_index] = (uint8_t *)space + block_index * block_size;
        pointer = (char *)(des_inode->location[location_index]);
    }
    else if (location_index < 72)
    {
        uint32_t **single_indirect;
        if (des_inode->location[8] == NULL)
        {
            single_indirect = (uint32_t **)((uint8_t *)space + block_index * block_size);
            des_inode->location[8] =(uint8_t *)space + block_index * block_size;

            block_index = superblock->first_vfree_bitmap_block;
            alloc_bitmap(block_index);
            *single_indirect = (uint32_t *)((uint8_t *)space + block_index * block_size);
            pointer = (char *) *single_indirect;
        }
        else
        {
            single_indirect = (uint32_t **)des_inode->location[8];
            single_indirect[location_index - 8] = (uint32_t *)((uint8_t *)space + block_index * block_size);
            pointer = (char *)single_indirect[location_index - 8];
        }
    }
    else
    {
        uint32_t ***double_indirect;
        if (des_inode->location[9] == NULL)
        {
            double_indirect = (uint32_t ***)((uint8_t *)space + block_index * block_size);
            des_inode->location[9] = (uint8_t *)space + block_index * block_size;

            block_index = superblock->first_vfree_bitmap_block;
            alloc_bitmap(block_index);
            *double_indirect = (uint32_t **)((uint8_t *)space + block_index * block_size);

            block_index = superblock->first_vfree_bitmap_block;
            alloc_bitmap(block_index);
            **double_indirect = (uint32_t *)((uint8_t *)space + block_index * block_size);
            pointer = (char *) **double_indirect;
        }
        else
        {
            double_indirect = (uint32_t ***)des_inode->location[9];

            int single_indirect_index = (location_index - 72) / 64;
            int block_i = (location_index - 72) % 64;

            if (double_indirect[single_indirect_index] == NULL)
            {
                double_indirect[single_indirect_index] = (uint32_t **)((uint8_t *)space + block_index * block_size);

                block_index = superblock->first_vfree_bitmap_block;
                alloc_bitmap(block_index);
                double_indirect[single_indirect_index][0] = (uint32_t *)((uint8_t *)space + block_index * block_size);
                pointer = (char *)double_indirect[single_indirect_index][0];
            }

            else
            {
                double_indirect[single_indirect_index][block_i] = (uint32_t *)((uint8_t *)space + block_index * block_size);
                pointer = (char *)double_indirect[single_indirect_index][block_i];
            }
        }
    }
    des_inode->offset[0]++;
    des_inode->offset[1] = 0;
    return pointer;
}


int fill_file(int des_index, int *position, char *content, int size, char mode)
{
    inode_t *des_inode;
    int block_index;
    int inside_block_index;
    char *fill_pointer;
    des_inode = &inode[des_index];
    int write_in = 0;	// number of character has written in

    if (mode == 'w' && !strcmp(des_inode->type, dir)) 
        return ERROR;

    if (des_inode->location[0] == NULL && alloc_block(des_inode) == ERROR)
        return ERROR;

    block_index = *position / block_size;
    inside_block_index = *position % block_size;
    
    if (block_index < 8)
        fill_pointer = (char *)(des_inode->location[block_index]);
    else if (block_index < 72)
        fill_pointer = (char *)(((uint32_t **)des_inode->location[8])[block_index - 8]);
    else
        fill_pointer = (char *)(((uint32_t ***)des_inode->location[9])[(block_index-72)/64][(block_index-72)%64]);

    fill_pointer += inside_block_index;

    while (write_in < size)
    {
        if (inside_block_index == block_size)
        {
            if ((fill_pointer = alloc_block(des_inode)) == ERROR)
                break;
            else
            {
                inside_block_index = 0;
                continue;
            }
        }

        *fill_pointer = *(content + write_in);
        fill_pointer++;
        write_in++;
        inside_block_index++;
        (*position)++;
    }

    if (des_inode->offset[0] > block_index)
        des_inode->offset[1] = inside_block_index;
    if (mode == 'c' && write_in == 16)
        des_inode->size += 16;                      // without '\0'
    else des_inode->size += write_in;               // without '\0'

    return write_in;

}



// initial inode offset[0] = -1
int alloc_inode(int parent_inode_index, char type[4])
{
    inode_t *new_inode;
    new_inode = inode + superblock->first_vfree_inode;

    strcpy(new_inode->type, type);
    new_inode->size = 0;
    new_inode->protaction = 0;
    new_inode->operation = 0x100;
    new_inode->parent_index = (uint16_t) parent_inode_index;
    new_inode->offset[0] = -1;
    new_inode->offset[1] = 0;

    // you also have to find the next available inode
    int i = superblock->first_vfree_inode + 1;
    for (; i < inode_list_num; i++)
        if (inode[i].operation == 0)
            break;
    superblock->vfreeInode--;
    superblock->first_vfree_inode = i;

    return FINE;
}


int scan(uint32_t *location[], int size, int *times, char list[][14])
{
    int i, j;		// for loop

    dirctory_entry_t *dirctory_entry;

    // you should search all the location....

    // search the first 8 directory location
    for (i = 0; i < size && *times > 0; i ++)
    {
        dirctory_entry = (dirctory_entry_t *)location[i];
        if (dirctory_entry == NULL)
            return ERROR;
        // one block is 256 bytes, and one dirctory entry is 16 bytes
        // thus one block could at most save 16 entry
        for (j = 0; j < 16 && *times > 0; j ++)
        {
            if ((dirctory_entry + j)->filename == NULL)
                return ERROR;

            strcpy(list[(*times) - 1], (dirctory_entry + j)->filename);
            (*times)--;
        }
    }

    return FINE;
}



int get_list(int des_index, char list[][14])
{
    inode_t *des_inode = &inode[des_index];
    int size = des_inode->size;
    int temp = size/16;
    int *times = &temp;
    int ret;

    if (*times <=0)
        return ERROR;

    if ((ret = scan(des_inode->location, 8, times, list)) == ERROR)
        return ERROR;
    if (*times <=0)
        return FINE;

    // search the single_indirect location
    uint32_t **single_indirect;

    if ((single_indirect = (uint32_t **)des_inode->location[8]) == NULL)
        return ERROR;

    if ((ret = scan(single_indirect, 64, times, list)) == ERROR)
        return ret;
    if (*times <=0)
        return FINE;

    // search the double_indirect location
    uint32_t ***double_indirect;
    if ((double_indirect = (uint32_t ***)des_inode->location[9]) == NULL)
        return ERROR;
    // a double_indirect pointer point to a table, contains 64 single_indirect pointer...
    int i;
    for(i = 0; i < 64; i++)
    {
        single_indirect = double_indirect[i];
        if (single_indirect == NULL)
            return ERROR;
        if ((ret = scan(single_indirect, 64, times, list)) == ERROR)
            return ret;
        if (*times <=0)
            return FINE;
    }
    return FINE;

}


int is_open(int des_index, FDT_t *FDT)
{
    file_object_t *file_object;
    file_object = FDT->file_object;

    while (file_object != NULL && file_object->inode_num != des_index)
        file_object = file_object->next;

    if (file_object == NULL)
        return FINE;
    else return ERROR;
}


int vfree_bitmap(char* block_location)
{
    int block_num;
    int bitmapByteIndex;
    int bitmapBitIndex;
    char* positionInBitmap;
    uint8_t temp1 = 0x08;
    uint8_t temp2 = 0xff;
    int nextIndex;

    block_num = (int) ((block_location - (char* )space) / block_size);

    bitmapByteIndex = block_num / 8;
    bitmapBitIndex  = block_num % 8;

    positionInBitmap = &(bitmap->index[bitmapByteIndex]);

    *positionInBitmap |= (temp2 ^ (temp1 >> bitmapBitIndex));

    superblock->vfreeBlock++;

    nextIndex = bitmapByteIndex * 8 + bitmapBitIndex;
    if (nextIndex < superblock->first_vfree_bitmap_block)
        superblock->first_vfree_bitmap_block = (uint16_t)nextIndex;
    return FINE;
}


int vfree_block(inode_t *des_inode)
{
    int location_index = des_inode->offset[0];
    char * blockPointer;
    int i, j;

    if (location_index < 0) // namely -1
        return FINE;

    if (location_index < 8)
    {
        for(i = 0; i <= location_index; i++)
        {

            if (des_inode->location[i] == NULL)
                break;
            memset(des_inode->location[i], 0, block_size);
            vfree_bitmap(des_inode->location[i]);
        }
    }

    else if (location_index < 72)
    {

        uint32_t **single_indirect;

        single_indirect = (uint32_t **)des_inode->location[8];

        // Iterate all 64 block pointers
        for (i = 0; i < 64; i++)
        {
            // Get block pointed to by indirect pointer
            blockPointer = (char *)single_indirect[i];
            // If this single indirect block has no more pointers to blocks...
            if (!blockPointer)
                break;
            memset(blockPointer, 0, block_size);
            vfree_bitmap(blockPointer);
        }
    }
    else
    {
        uint32_t ***double_indirect;

        double_indirect = (uint32_t ***)des_inode->location[9];

        for (i = 0; i < 64; i++)
        {

            if (!double_indirect[i])
                break;
            // Iterate all 64 block pointers
            for (j = 0; j < 64; j++)
            {
                blockPointer = (char *)double_indirect[i][j];
                if (!blockPointer)
                    break;
                memset(blockPointer, 0, block_size);
                vfree_bitmap(blockPointer);
                // Mark the block bitmap.
            }
        }
    }
    return FINE;
}


char * block_address(inode_t *des_inode, int block_index, int inside_block_index)
{
    if (block_index < 8)
        return ((char *)(des_inode->location[block_index]) + inside_block_index);
    if (block_index < 72)
    {
        uint32_t **single_indirect = (uint32_t **)des_inode->location[8];
        return ((char *)(single_indirect[block_index - 8]) + inside_block_index);
    }
    else
    {
        uint32_t ***double_indirect = (uint32_t ***)des_inode->location[9];
        return ((char *)(double_indirect[(block_index - 72)/64][(block_index - 72)%64]) + inside_block_index);
    }
}




int vfree_parentcontent(char *filename, int parent_inode_index)
{
    int *temp;
    int position = -1;
    temp = &position;

    dirctory_entry_t *dirctory_entry;
    dirctory_entry_t *last_dirctory_entry;

    inode_t *parent_inode = &inode[parent_inode_index];
    int block_index;
    int inside_block_index;

    int last_block_index;
    int last_inside_block_index;

    is_exist(parent_inode_index, filename, temp);

    block_index = position / 16;
    inside_block_index = (position % 16) * 16;

    last_block_index = parent_inode->offset[0];
    last_inside_block_index = parent_inode->offset[1] - 16;

    dirctory_entry = (dirctory_entry_t *)block_address(parent_inode, block_index, inside_block_index);
    last_dirctory_entry = (dirctory_entry_t *)block_address(parent_inode,last_block_index,last_inside_block_index);

    strcpy(dirctory_entry->filename, last_dirctory_entry->filename);
    dirctory_entry->inode_index = last_dirctory_entry->inode_index;

    parent_inode->offset[1] -= 16;
    parent_inode->size -= 16;
    return FINE;
}


int vfree_inode(char *filename, int des_index)
{
    inode_t *des_inode = &inode[des_index];

    vfree_block(des_inode);

    vfree_parentcontent(filename, des_inode->parent_index);

    strcmp(des_inode->type, nul);
    des_inode->size = 0;
    des_inode->protaction = 0;
    des_inode->operation = 0x0000;
    des_inode->parent_index = 0;
    des_inode->offset[0] = 0;
    des_inode->offset[1] = 0;
    // for (i = 0;i < 10; i++)
    // 	(void *)des_index->location[i] = NULL;

    // vfree the block it hold and bitmap
    // for the bitmap, you just use the address of the block to compute
    // the place of bit.

    // for block when you delete, put the last file cover the delete place
    // when adjust your offset pointer.



    superblock->vfreeInode++;
    if (des_index < superblock->first_vfree_inode)
        superblock->first_vfree_inode = (uint16_t)des_index;

    return FINE;
}

char * get_blockpointer(int position, inode_t *des_inode)
{ 
    int block_index;
    int inside_block_index;
    uint32_t **single_indirect;
    uint32_t ***double_indirect;

    block_index = position / block_size;
    inside_block_index = position % block_size;

    if (position < 0)
        return ERROR;

    if (block_index < 8)
        return (char *)(des_inode->location[block_index]) + inside_block_index;
    else if (block_index < 72)
    {
        single_indirect = (uint32_t **)des_inode->location[8];
        return (char *)(single_indirect[block_index - 8]) + inside_block_index;
    }
    else if (block_index < 4167) 
    {
        double_indirect = (uint32_t ***)des_inode->location[9];
        return (char *)(double_indirect[(block_index - 72)/64][(block_index-72)%64]) + inside_block_index;
    }
    return ERROR;
}





