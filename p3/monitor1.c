#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include "monitor.h"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int main(void){
    int f, sig, *check, *t;
    char *input, cwd[256];
    check = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    t = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    input = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    *check = 0;
    *t = time(NULL);
    f = fork();
    if(f == 0){
        DIR *dir;
        struct dirent *entry;
        struct stat sb;
        int r;
        while(1){
            for(sig = 1; sig < 65; sig++){
                signal(sig, save);
            }
            *t = time(NULL);
            getcwd(cwd, 256);
            fprintf(stderr, ANSI_COLOR_CYAN  "stat prog %s$ "  ANSI_COLOR_RESET, cwd);
            scanf("%s", input);
            if(strcmp(input, "list") == 0){
                dir = opendir(".");
                while((entry = readdir(dir)) != NULL){
                    printf("%s \n", entry->d_name);
                }
            } else if(strcmp(input, "q") == 0){
                if(*check == 1){
                    kill(getpid(), SIGKILL);
                }
                *check = 1;
                exit(0);
            } else if(strcmp(input, "..") == 0){
                chdir("..");
            } else if(input[0] == '/'){
                if(chdir(input+1) != 0){
                    printf("%s does not exist\n", input);
                }
            } else{
                r = stat(input,&sb);
                if(r!=0)
                {
                    perror(__FUNCTION__);
                    continue;
                }
                else
                {
                    printf("could locate file \n");
                    printFileStats(sb);
                }
            }
        }
    } else{
        while(1){
            for(sig = 1; sig < 65; sig++){
                signal(sig, save);
            }
            if(*check == 1){
                kill(f, SIGKILL);
                munmap(check, sizeof(int));
                munmap(t, sizeof(int));
                munmap(input, 256);
                return 0;
            }
        }
        while(wait(0) != 0){
            f = fork();
            if(f == 0){
                fprintf(stderr, "childpid: %d", getpid());
                DIR *dir;
                struct dirent *entry;
                struct stat sb;
                int r;
                while(1){
                    for(sig = 1; sig < 65; sig++){
                        signal(sig, save);
                    }
                    *t = time(NULL);
                    getcwd(cwd, 256);
                    fprintf(stderr, ANSI_COLOR_CYAN  "stat prog %s$ "  ANSI_COLOR_RESET, cwd);
                    fflush(stdin);
                    scanf("%s", input);
                    fflush(stdin);
                    if(strcmp(input, "list") == 0){
                        dir = opendir(".");
                        while((entry = readdir(dir)) != NULL){
                            printf("%s \n", entry->d_name);
                        }
                    } else if(strcmp(input, "q") == 0){
                        if(*check == 1){
                            kill(getpid(), SIGKILL);
                        }
                        *check = 1;
                        exit(0);
                    } else if(strcmp(input, "..") == 0){
                        chdir("..");
                    } else if(input[0] == '/'){
                        if(chdir(input+1) != 0){
                            printf("%s does not exist\n", input);
                        }
                    } else{
                        r = stat(input,&sb);
                        if(r!=0)
                        {
                            perror(__FUNCTION__);
                            continue;
                        }
                        else
                        {
                            printf("could locate file \n");
                            printFileStats(sb);
                        }
                    }
                }
            }
        }
    }
}

/*got from the filestats.c example code on canvas*/
void printFileStats(struct stat sb){
     printf("----------------------------------------------------------\n");
     printf("ID of containing device:  [%jx,%jx]\n",
                   (uintmax_t) major(sb.st_dev),
                   (uintmax_t) minor(sb.st_dev));

           printf("File type:                ");

           switch (sb.st_mode & S_IFMT) {
           case S_IFBLK:  printf("block device\n");            break;
           case S_IFCHR:  printf("character device\n");        break;
           case S_IFDIR:  printf("directory\n");               break;
           case S_IFIFO:  printf("FIFO/pipe\n");               break;
           case S_IFLNK:  printf("symlink\n");                 break;
           case S_IFREG:  printf("regular file\n");            break;
           case S_IFSOCK: printf("socket\n");                  break;
           default:       printf("unknown?\n");                break;
           }

           printf("I-node number:            %ju\n", (uintmax_t) sb.st_ino);

           printf("Mode:                     %jo (octal)\n",
                   (uintmax_t) sb.st_mode);

           printf("Link count:               %ju\n", (uintmax_t) sb.st_nlink);
           printf("Ownership:                UID=%ju   GID=%ju\n",
                   (uintmax_t) sb.st_uid, (uintmax_t) sb.st_gid);

           printf("Preferred I/O block size: %jd bytes\n",
                   (intmax_t) sb.st_blksize);
           printf("File size:                %jd bytes\n",
                   (intmax_t) sb.st_size);
           printf("Blocks allocated:         %jd\n",
                   (intmax_t) sb.st_blocks);

           printf("Last status change:       %s", ctime(&sb.st_ctime));
           printf("Last file access:         %s", ctime(&sb.st_atime));
           printf("Last void printFileStats(struct stat sb)file modification:   %s", ctime(&sb.st_mtime));
             printf("----------------------------------------------------------\n\n");

}

void save(int i){

}