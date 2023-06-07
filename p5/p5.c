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
#define MATRIX_DIMENSION_XY 10

int main(int argc, char *argv[]){
    char name[50], *par_count, i, temp, f;
    sprintf(name, "./%s", argv[1]);
    par_count = argv[2];
    temp = atoi(par_count);
    for(i = 0; i < temp; i++){
        char par_id[sizeof(int)];
        sprintf(par_id, "%d", i);
        f = fork();
        if(f == 0){
            char *args[] = {name, par_id, par_count, NULL};
            execv(args[0], args);
        }
    }
    for(i = 0; i < temp; i++){
        wait(0);
    }
    return 0;
}