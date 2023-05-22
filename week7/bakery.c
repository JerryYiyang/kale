#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
/*read up on the bakery algorithm using geeksforgeeks*/

volatile int *ticket, *entering;

void lock(int p);
void unlock(int p);
int max(int x, int y);

int main(){
    char *shared;
    int f;
    char s1[] = "Leave the mixing to the dj's! Don't forget to take care of yourself and to drink a lot of water.\n";
    char s2[] = "Hello world! This is a classic sentence used for computer science language courses.\n";
    shared = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    ticket = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    entering = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    f = fork();
    if(f == 0){
        int temp = 0;
        while(1){
            lock(0);
            if(temp == 0){
                strcpy(shared, s1);
                temp = 1;
            } else{
                strcpy(shared, s2);
                temp = 0;
            }
            unlock(0);
        }
    } else{
        while(1){
            lock(1);
            puts(shared);
            unlock(1);
        }
    }
}

void lock(int p){
/*p = 0 for child, 1 for parent*/
    int j;
    entering[p] = 1;
    ticket[p] = 1 + max(ticket[0], ticket[1]);
    entering[p] = 0;
    for(j = 0; j < 2; ++j){
        if (j == p) continue;
        while(entering[j]){}
        while(ticket[j] != 0 && (ticket[j] < ticket[p] || (ticket[j] == ticket[p] && j < p))){}
    }
}

void unlock(int p){
    ticket[p] = 0;
}

int max(int x, int y){
    return x > y ? x : y;
}
