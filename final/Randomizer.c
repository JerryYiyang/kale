#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

int main(int argc, char *argv[]){
    int n, i, j, temp, f, fd[2];
    char *p, *nums;
    n = atoi(argv[1]);
    p = argv[2];
    pipe(fd);
    f = fork();
    if(f == 0){
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO); 
        execlp("./EvenOdd", "./EvenOdd", argv[2], NULL);
    } else{
        close(fd[0]);
        nums = malloc(sizeof(char) * 3 * n);
        srand(time(NULL));
        j = 0;
        for (i = 0; i < n; i++) {
            temp = rand() % 11;
            if (rand() % 100 < 20){
                temp = -temp;
            }
            j += sprintf(nums + j, "%d ", temp);
        }
        nums[j - 1] = '\n';
        write(fd[1], nums, strlen(nums));
        close(fd[1]);
        free(nums);
        wait(0);
    }
    return 0;
}
