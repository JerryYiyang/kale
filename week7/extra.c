#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

volatile int *flag, *turn;

int main(){
    char *shared;
    int f;
    char s1[] = "Leave the mixing to the dj's! Don't forget to take care of youreslf and to drink a lot of water.\n";
    char s2[] = "Hello world! This a classic sentence used for computer science language courses.\n";
    shared = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    flag = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    turn = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    f = fork();
    if(f == 0){
        int temp = 0;
        while(1){
            if(temp == 0){
                strcpy(shared, s1);
                temp = 1;
            } else{
                strcpy(shared, s2);
                temp = 0;
            }
        }
    } else{
        while(1){
            puts(shared);
        }
    }
}