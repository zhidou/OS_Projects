# include "fileOperation.h"

static struct file_operations ramdisk_operations;


static struct proc_dir_entry *proc_entry;
superblock_t *superblock;
inode_t *inode;
bitmap_t *bitmap;
static void *ramdisk;
void *space;
//static char pwd[100];		// print working directory

extern int destroy_FDT(void);

void initSuperblock(void)
{
    superblock->vfreeBlock = allocatable_space_size - 1;
    superblock->vfreeInode = inode_list_num - 1;
    superblock->first_vfree_bitmap_block = 1;
    superblock->first_vfree_inode = 1;
}

void initBitmap(void)
{
    int i ;

    bitmap->index[0] |= 0x80;

    for (i = 1; i < bitmap_sublock; i++)
        bitmap->index[i] &= 0;
}

void initInode(void)
{
    strcpy(inode->type, dir);
    inode->size = 0;
    inode->location[0] = space;
    inode->protaction = 0;
    inode->operation = 0x100;
    inode->parent_index = -2;
    inode->offset[0] = 0;
    inode->offset[1] = 0;
}

int initRamdisk(void)
{
    printk("initialize Ramdisk!\n");
    ramdisk = (void *) vmalloc(ramdisk_size);

    superblock = (superblock_t *) ramdisk;
    inode = (inode_t *) ((char *)ramdisk + block_size);
    bitmap = (bitmap_t *) ((char *)inode + block_size * inode_list_size);
    space = (void *) ((char *)bitmap + block_size * bitmap_size);

    memset((void *) ramdisk, 0, ramdisk_size);

    // we initalize them with the root directory file "/"
    initSuperblock();
    initBitmap();
    initInode();

    return FINE;
}

static int ramdisk_ioctl(struct inode *inode, struct file *file, 
                         unsigned int cmd, unsigned long arg)
{
	char cur_path[100];
	char (*list)[14];
	char *pathname;
	int ret = ERROR;
	int size;
	int *fd;
	file_info_t *file_info;
	dirctory_entry_t *dirctory_entry;
	switch (cmd)
	{
		case IOCTL_PWD:
			strcpy(cur_path, rd_pwd());
			ret = copy_to_user((char *)arg, cur_path, sizeof(cur_path));
			return ret;
		case IOCTL_LS:
			list = (char *)vmalloc(sizeof(char) * 1024 * 14);
			memset((char *) list, 0, 1024 * 14);
			if ((ret = rd_ls(list)) != ERROR )
				ret = copy_to_user((char **)arg, list, 1024 * 14);
			vfree(list);
			return ret;
		case IOCTL_CREAT:
			size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char*)vmalloc(size);
            copy_from_user(pathname, (char *)arg, size);
            ret = rd_creat(pathname);
            vfree(pathname);
            return ret;
		case IOCTL_MKDIR:
		 	size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char*)vmalloc(size);
            copy_from_user(pathname, (char *)arg, size);
            ret = rd_mkdir(pathname);
            vfree(pathname);
			return ret;
		case IOCTL_UNLINK:
			size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char*)vmalloc(size);
            copy_from_user(pathname, (char *)arg, size);
			ret = rd_unlink(pathname);
            vfree(pathname);
            return ret;
        case IOCTL_OPEN:
			size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char*)vmalloc(size);
            copy_from_user(pathname, (char *)arg, size);
            ret = rd_open(pathname);
            vfree(pathname);
			return ret;
		case IOCTL_CLOSE:
			size = sizeof(int);
			fd = (int *)vmalloc(size);
            copy_from_user(fd, (int *)arg, size);
            ret = rd_close(*fd);
			return ret;
		case IOCTL_WRITE:
			size = sizeof(file_info_t);
			file_info = (file_info_t *)vmalloc(size);
            copy_from_user(file_info, (file_info_t *)arg, size);
            ret = rd_write(file_info->fdt_index, file_info->input, file_info->size);
            vfree(file_info);
			return ret;
		case IOCTL_READ:
			size = sizeof(file_info_t);
			file_info = (file_info_t *)vmalloc(size);
            copy_from_user(file_info, (file_info_t *)arg, size);
            ret = rd_read(file_info->fdt_index, file_info->input, file_info->size);
            copy_to_user((file_info_t *)arg, file_info, size);
            vfree(file_info);
			return ret;
		case IOCTL_READDIR:
			size = sizeof(file_info_t);
			file_info = (file_info_t *)vmalloc(size);
            copy_from_user(file_info, (file_info_t *)arg, size);
            ret = rd_readdir(file_info->fdt_index, file_info->input);
            dirctory_entry = (dirctory_entry_t *)file_info->input;
            copy_to_user((file_info_t *)arg, file_info, size);
            vfree(file_info);
			return ret;
		case IOCTL_LSEEK:
			size = sizeof(file_info_t);
			file_info = (file_info_t *)vmalloc(size);
            copy_from_user(file_info, (file_info_t *)arg, size);
            ret = rd_lseek(file_info->position, file_info->fdt_index);
            vfree(file_info);
		default:
			break;
	}

	return ret;
}


void destroy_ramdisk(void)
{
	destroy_FDT();
	vfree(ramdisk);
	remove_proc_entry("Ramdisk", NULL);
}

static int __init initialization_ramdisk(void) 
{
	printk("Loading ramdisk module\n");

	ramdisk_operations.ioctl = ramdisk_ioctl;

	proc_entry = create_proc_entry("Ramdisk", 0444, NULL);

	if (!proc_entry)
	{
		printk("Error creating Ramdisk proc file \n");
		return ERROR;
	}
	
	proc_entry->proc_fops = &ramdisk_operations;

	if (initRamdisk() == ERROR)
	{	printk("Error initialize ramdisk\n");
		return ERROR;
	}

	return FINE;
}

static void __exit cleanup_ramdisk(void)
{
	printk("Remove Ramdisk module\n");
	destroy_ramdisk();
}

module_init(initialization_ramdisk);
module_exit(cleanup_ramdisk);