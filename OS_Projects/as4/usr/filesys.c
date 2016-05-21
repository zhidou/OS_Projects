# include "filesys.h"

int main(void)
{
    char cmd[15] = "test";
    char input[100];
    char output[100];

    /* for test use */
    char data1[DIRECT*BLK_SZ]; /* Largest data directly accessible */
    char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
    char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
    char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */
    char data4[2 * BLK_SZ];

    int retval;
    int fd;

    /* Some arbitrary data for our files */
    memset (data1, '1', sizeof (data1));
    memset (data2, '2', sizeof (data2));
    memset (data3, '3', sizeof (data3));
    memset (data4, '4', sizeof (data4));
    data1[DIRECT*BLK_SZ - 1] = '\0';
    data2[PTRS_PB*BLK_SZ - 1] = '\0';
    data3[PTRS_PB*PTRS_PB*BLK_SZ - 1] = '\0';
    data4[2*BLK_SZ - 1] = '\0';

    //memset (output, '\0', sizeof(output));
    printf("Start Z-Ramdisk!\n");
    while (strcmp(cmd, "exit"))
    {
        printf("Ramdisk # ");
        fgets(cmd, 15, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        if (!strcmp(cmd, "exit"));
        else if (!strcmp(cmd, "pwd"))
            rd_pwd();
        else if (!strcmp(cmd, "ls"))
            rd_ls();
        else if (!strncasecmp(cmd, "mkdir", 5))
        {
            int i = 5;
            while (cmd[i] == ' ' && cmd[i] != '\0')
                i++;
            if (cmd[i] == '\0')
                continue;
            char pathname[100];
            strcpy(pathname, cmd + i);
            rd_mkdir(pathname);
            rd_ls();
        }
        else if (!strncasecmp(cmd, "creat", 5))
        {
            int i = 5;
            while (cmd[i] == ' ' && cmd[i] != '\0')
                i++;
            if (cmd[i] == '\0')
                continue;
            char pathname[100];
            strcpy(pathname, cmd + i);
            rd_creat(pathname);
            rd_ls();
        }
        else if (!strncasecmp(cmd, "open", 4))
        {
            int i = 4;
            while (cmd[i] == ' ' && cmd[i] != '\0')
                i++;
            if (cmd[i] == '\0')
                continue;
            char pathname[100];
            strcpy(pathname, cmd + i);
            printf("Open file: %s\n", pathname);
            printf("fd_ID: %d\n", rd_open(pathname));
        }
        else if (!strcmp(cmd, "cd ../"))
            rd_close(0);
        else if (!strncasecmp(cmd, "cd", 2))
        {
            int i = 2;
            while (cmd[i] == ' ' && cmd[i] != '\0')
                i++;
            if (cmd[i] == '\0')
                continue;
            char pathname[100];
            strcpy(pathname, cmd + i);
            rd_open(pathname);
        }
        else if (!strncasecmp(cmd, "rm", 2))
        {

            int i = 2;
            while (cmd[i] == ' ' && cmd[i] != '\0')
                i++;
            if (cmd[i] == '\0')
                continue;
            char pathname[100];
            strcpy(pathname, cmd + i);
            rd_unlink(pathname);
        }
        else if (!strcmp(cmd, "close"))
        {
            int fd;
            printf("fd_ID: ");
            scanf("%d",&fd);
            getchar();
            
            rd_close(fd);
        }
        else if (!strncasecmp(cmd, "write", 5))
        {
            printf("Inpute content: ");
            int fd;
            fgets(input, 100, stdin);
            input[strlen(input) - 1] = '\0';
            printf("fd_ID: ");
            scanf("%d",&fd);
            getchar();
            int size = strlen(input);

            rd_write(fd, input, size);
        }
        else if (!strncasecmp(cmd, "read", 4))
        {
            int fd;
            int size;
            printf("fd_ID: ");
            scanf("%d",&fd);
            getchar();
            printf("read number: ");
            scanf("%d",&size);
            getchar();

            rd_read(fd, output, size);
        }
        else if (!strcmp(cmd, "readdir"))
        {
            int fd;
            printf("fd number: ");
            scanf("%d",&fd);
            getchar();

            rd_readdir(fd, output);
        }
        else if (!strncasecmp(cmd, "lseek", 5))
        {
            int fd;
            int position;
            printf("fd_ID: ");
            scanf("%d",&fd);
            getchar();
            printf("position: ");
            scanf("%d", &position);
            getchar();
            rd_lseek(position, fd);
        }
        else if (!strcmp(cmd, "test"))
        {
            // create big file
            retval = rd_creat(PATH_PREFIX "/bigfile");

            if (retval < 0) {
                fprintf (stderr, "creat: File creation error! status: %d\n",
                         retval);
                exit(EXIT_FAILURE);
            }

            retval = rd_open(PATH_PREFIX "/bigfile.txt"); /* Open file to write to it */

            if (retval < 0) {
                fprintf (stderr, "open: File open error! status: %d\n",
                         retval);

                exit(EXIT_FAILURE);
            }

            fd = retval;			/* Assign valid fd */

            // write big file
            retval = rd_write(fd, testdata, sizeof(testdata));

            if (retval < 0) {
                fprintf (stderr, "write: File write STAGE3 error! status: %d\n",
                         retval);

                exit(EXIT_FAILURE);
            }
            
            printf("finished write!!\n");
            // read big file
            
            retval = rd_read(fd, addr, sizeof(testdata));

            if (retval < 0) {
                fprintf (stderr, "read: File read STAGE2 error! status: %d\n",
                         retval);

                exit(EXIT_FAILURE);
            }
            /* Should be all 2s here... */
            printf("%d\n", strlen(testdata));
            printf("%d\n", strlen(addr));
            return 0;
        }
        else
            printf("%s: command not found\n", cmd);
    }

    return 0;

}