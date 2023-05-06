#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>

void save(int i);

int main(void){
    int f, sig;
    time_t T = time(NULL);
    struct tm tm = *localtime(&T); //tm.tm_hour, tm.tm_min
    DIR *dir = opendir(".");
    struct dirent *entry;
    fprintf(stderr, "Current pid: %d\n", getpid());
    f = fork();
    if(f == 0){
        fprintf(stderr, "Child's pid: %d\n", getpid());
        for(sig = 1; sig < 65; sig++){
            signal(sig, save);
        }
        if(dir == NULL){
            printf("Could not open dir\n");
            exit(1);
        }
        while((entry = readdir(dir)) != NULL){
            printf("%s \n", entry->d_name);
        }
        while(1){
            printf("Current time: %d:%d\n", tm.tm_hour, tm.tm_min);
            sleep(10);
        }
    } else{
        fprintf(stderr, "Parent pid: %d\n", getpid());
        for(sig = 1; sig < 65; sig++){
            signal(sig, save);
        }
        if(waitpid(f, NULL, WNOHANG) != 0){
            f = fork();
            if(f == 0){
                fprintf(stderr, "Child's pid: %d\n", getpid());
                for(sig = 1; sig < 65; sig++){
                    signal(sig, save);
                }
                if(dir == NULL){
                    printf("Could not open dir\n");
                    exit(1);
                }
                while((entry = readdir(dir)) != NULL){
                    printf("%s \n", entry->d_name);
                }
                while(1){
                    printf("Current time: %d:%d\n", tm.tm_hour, tm.tm_min);
                    sleep(10);
                }
            }
        }
    }
}

void save(int i){
    printf("I'm saved!\n");
}