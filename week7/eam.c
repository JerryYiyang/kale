#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

volatile int *flag, *turn;
/*for flag: 0 = idle, 1 = waiting, 2 = active*/
/*read up on eisenberg and mcguire on wikepedia*/
void enter(int p);
void leave(int p);

int main(){
    char *shared;
    int f;
    char s1[] = "Leave the mixing to the dj's! Don't forget to take care of youreslf and to drink a lot of water.\n";
    char s2[] = "Hello world! This a classic sentence used for computer science language courses.\n";
    shared = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    flag = mmap(NULL, sizeof(int) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    turn = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    f = fork();
    *turn = 0;
    if(f == 0){
        int temp = 0;
        while(1){
            enter(0);
            if(temp == 0){
                strcpy(shared, s1);
                temp = 1;
            } else{
                strcpy(shared, s2);
                temp = 0;
            }
            leave(0);
        }
    } else{
        while(1){
            enter(1);
            puts(shared);
            leave(1);
        }
    }
}

void enter(int p){
/*i = 0 for child, 1 for parent*/
    int index;
    do{
        flag[p] = 1;
        index = *turn;
        while(index != p){
            if(flag[index] != 0){
                index = *turn;
            } else{
                index = (index + 1) % 2;
            }
        }
        flag[p] = 2;
        index = 0;
        while((index < 2) && ((index == p) || (flag[index] != 2))){
            index++;
        }
    } while(!(index >= 2) && ((*turn == p) || (flag[*turn] == 0)));
    *turn = p;
}

void leave(int p){
    int index = (*turn + 1) % 2;
    while(flag[index] == 0){
        index = (index + 1) % 2;
    }
    *turn = index;
    flag[p] = 0;
}