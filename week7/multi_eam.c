#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

volatile int flag[2] = {0, 0};
volatile int turn = 0;

char shared[1000];
char s1[] = "Leave the mixing to the dj's! Don't forget to take care of yourself and to drink a lot of water.\n";
char s2[] = "Hello world! This is a classic sentence used for computer science language courses.\n";

void* child_thread(void* arg) {
    int temp = 0;
    while (1) {
        flag[0] = 1;
        turn = 1;
        while (flag[1] == 1 && turn == 1);
        // Critical Section
        strcpy(shared, temp == 0 ? s1 : s2);
        temp = 1 - temp;
        flag[0] = 0;
    }
    pthread_exit(NULL);
}

void* parent_thread(void* arg) {
    int temp = 0;
    while (1) {
        flag[1] = 1;
        turn = 0;
        while (flag[0] == 1 && turn == 0);
        // Critical Section
        printf("%s", shared);
        temp = 1 - temp;
        flag[1] = 0;
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t child, parent;
    pthread_create(&child, NULL, child_thread, NULL);
    pthread_create(&parent, NULL, parent_thread, NULL);

    pthread_join(child, NULL);
    pthread_join(parent, NULL);

    return 0;
}
