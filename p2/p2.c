#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "p2.h"
#define PAGESIZE 4096

chunkhead *head = NULL;

int main(void){
    unsigned char *a[100];
    clock_t ca, cb;
    ca = clock();
    for(int i=0;i<100;i++)
    a[i]= mymalloc(1000);
    for(int i=0;i<90;i++)
    myfree(a[i]);
    myfree(a[95]);
    a[95] = mymalloc(1000);
    for(int i=90;i<100;i++)
    myfree(a[i]);
    cb = clock();
    printf("\nduration: % f\n", (double)(cb - ca));
    return 0;
}

unsigned char *mymalloc(unsigned int size){
    if(size % PAGESIZE != 0) { /*O(1)*/
        size = ((int)(size / PAGESIZE) + 1) * PAGESIZE;
    }
    if(head == NULL){ /*O(1)*/
        void *check;
        head = (chunkhead *)sbrk(0);
        check = sbrk(sizeof(chunkhead) + size);
        if(check == (void *)-1){ /*O(1)*/
            return NULL;
        }
        head->size = size;
        head->info = 1;
        head->prev = NULL;
        head->next = NULL;
        return (unsigned char *)head + sizeof(chunkhead);
    } else{
        chunkhead *curr = (chunkhead *)head;
        chunkhead *prev;
        /*maybe to make this faster i can do multiple processes and have head's prev point to the end of the chunks*/
        while(curr != NULL) { /*O(n)*/
            if(curr->info == 0){
                curr->info = 1;
                /*splitting chunks if needed*/
                if(curr->size > size){ /*O(1)*/
                    chunkhead *open;
                    int remainder = curr->size - size - sizeof(chunkhead);
                    curr->size = size;
                    open = (chunkhead *)((unsigned char *)curr + size + sizeof(chunkhead));
                    open->info = 0;
                    open->size = remainder;
                    open->next = curr->next;
                    open->prev = (unsigned char *)curr;
                    curr->next = (unsigned char *)open;
                }
                return (unsigned char *)curr + sizeof(chunkhead);
            }
            prev = curr;
            curr = (chunkhead *)curr->next;
        }
        curr = (chunkhead *)sbrk(sizeof(chunkhead) + size);
        if(curr == (void *)-1){ /*O(1)*/
            return NULL;
        }
        curr->info = 1;
        curr->size = size;
        curr->next = NULL;
        curr->prev = (unsigned char *)prev;
        prev->next = (unsigned char *)curr;
        return (unsigned char *)curr + sizeof(chunkhead);
    }
    return NULL;
}

void myfree(unsigned char *address){
    chunkhead *curr = (chunkhead *)(address - sizeof(chunkhead));
    if(curr->next == NULL){ /*O(1)*/
        sbrk(-(sizeof(chunkhead) + curr->size));
    } else{ /*O(1)*/
        chunkhead *temp;
        curr->info = 0;
        if(curr->prev != NULL && ((chunkhead *)curr->prev)->info == 0){ /*O(1)*/
            temp = (chunkhead *)curr->prev;
            temp->next = (unsigned char *)curr->next;
            if(temp->next != NULL){ /*O(1)*/
                ((chunkhead *)temp->next)->prev = (unsigned char *)temp;
            }
            temp->size += curr->size + sizeof(chunkhead);
        }
        if(curr->next != NULL && ((chunkhead *)curr->next)->info == 0){ /*O(1)*/
            temp = (chunkhead *)curr->next;
            curr->next = (unsigned char *)temp->next;
            if(curr->next != NULL){ /*O(1)*/
                ((chunkhead *)curr->next)->prev = (unsigned char *)curr;
            }
            curr->size += temp->size + sizeof(chunkhead);
        }
    }
}

void analyse(){
    chunkhead *curr = head;
    int count = 0;
    while(curr != NULL){
        printf("Chunk #%d\n", count);
        printf("Size = %u bytes\n", curr->size);
        if(curr->info == 0){
            printf("Free\n");
        } else{
            printf("Occupied\n");
        }
        printf("Next = %p\n", curr->next);
        printf("Prev = %p\n", curr->prev);
        curr = (chunkhead*)curr->next;
        count += 1;
    }
}