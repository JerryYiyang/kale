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


/*doesnt work if im calling from a.out for some reason*/
void oddEven(int arr[], int n, int id, int p);
void swap(int arr[], int i, int j, int *sorted);
void synch(int par_id,int par_count,int *ready);

int *ready;

int main(int argc, char *argv[]){
    char num[10000], *token;
    int p, i, f, *nums, n, count;
    read(STDIN_FILENO, num, 10000);
    p = atoi(argv[1]);
    n = strlen(num) / 2;
    nums = mmap(NULL, sizeof(int) * n, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    token = strtok(num, " ");
    count = 0;
    while(token != NULL){
        nums[count] = atoi(token);
        token = strtok(NULL, " ");
        count++;
    }
    printf("initial array: [ ");
    for(i = 0; i < count; i++){
        if(i != (count - 1)){
            printf("%d, ", nums[i]);
        }
    }
    printf("%d ]\n", nums[count - 1]);
    if((count / p) < 2){
        p = count / 2;
    }
    ready = mmap(NULL, sizeof(int) * p, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    for(i = 0; i < p; i++){
        ready[i] = 0;
    }
    for(i = 0; i < p; i++){
        f = fork();
        if(f == 0){
            oddEven(nums, count, i, p);
            exit(0);
        }
    }
    for(i = 0; i < p; i++){
        wait(0);
    }
    printf("sorted array:  [ ");
    for(i = 0; i < count; i++){
        if(i != (count - 1)){
            printf("%d, ", nums[i]);
        }
    }
    printf("%d ]\n", nums[count - 1]);
    printf("processes: %d\n", p);
    munmap(ready, sizeof(int) * p);
    munmap(nums, sizeof(int) * n);
    return 0;
}

void oddEven(int arr[], int n, int id, int p){
    int sorted = 0;
    while (!sorted) {
        sorted = 1;
        synch(id, p, ready);
        for (int i = 1; i <= n - 2; i += 2){
            if(i % p == id){
                swap(arr, i, i + 1, &sorted);
            }
        }
        synch(id, p, ready);
        for (int i = 0; i <= n - 2; i += 2){
            if(i % p == id){
                swap(arr, i, i + 1, &sorted);
            }
        }
    }
}

void swap(int arr[], int i, int j, int *sorted){
    if (arr[i] > arr[j]) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        *sorted = 0;
    }
}

void synch(int par_id,int par_count,int *ready)
{
    int i;
    ready[par_id] += 1;
    for(i = 0; i < par_count; i++){
        while(ready[par_id] > ready[i]);
    }
}
