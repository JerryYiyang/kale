#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "p2.h"
#define PAGESIZE 4096
#define _GNU_SOURCE

chunkhead *head = NULL;
freechunk *freeList;

int main(void){
// byte*a[100];
// analyze(); //50% points
// for(int i=0;i<100;i++)
// a[i]= mymalloc(1000);
// for(int i=0;i<90;i++)
// myfree(a[i]);
// analyze(); //50% of points if this is correct
// myfree(a[95]);
// a[95] = mymalloc(1000);
// analyze();//25% points, this new chunk should fill the smaller free one
// //(best fit)
// for(int i=90;i<100;i++)
// myfree(a[i]);
// analyze();// 25% should be an empty heap now with the start address
// //from the program start

    // byte *a[100];
    // clock_t ca, cb;
    // ca = clock();
    // for(int i=0;i<100;i++)
    // a[i]= mymalloc(1000);
    // for(int i=0;i<90;i++)
    // myfree(a[i]);
    // myfree(a[95]);
    // a[95] = mymalloc(1000);
    // for(int i=90;i<100;i++)
    // myfree(a[i]);
    // cb = clock();
    // printf("\nduration: % f\n", (double)(cb - ca));

    unsigned char *a,*b,*c;
    a = mymalloc(1000);
    b = mymalloc(1000);
    c = mymalloc(1000);
    myfree(b);
    myfree(c);
    analyse();
    return 0;
}

byte *mymalloc(unsigned int size){
    if((size + sizeof(chunkhead)) % PAGESIZE != 0) { /*O(1)*/
        size = ((int)((size + sizeof(chunkhead)) / PAGESIZE) + 1) * PAGESIZE;
    }
    if(head == NULL){ /*O(1)*/
        void *check;
        head = (chunkhead *)sbrk(0);
        check = sbrk(size);
        if(check == (void *)-1){ /*O(1)*/
            return NULL;
        }
        head->size = size;
        head->info = 1;
        head->prev = NULL;
        head->next = NULL;
        return (byte *)head + sizeof(chunkhead);
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
                    int remainder = curr->size - size;
                    curr->size = size;
                    open = (chunkhead *)((byte *)curr + size);
                    open->info = 0;
                    open->size = remainder;
                    open->next = curr->next;
                    open->prev = (byte *)curr;
                    curr->next = (byte *)open;
                }
                return (byte *)curr + sizeof(chunkhead);
            }
            prev = curr;
            curr = (chunkhead *)curr->next;
        }
        curr = (chunkhead *)sbrk(size);
        if(curr == (void *)-1){ /*O(1)*/
            return NULL;
        }
        curr->info = 1;
        curr->size = size;
        curr->next = NULL;
        curr->prev = (byte *)prev;
        prev->next = (byte *)curr;
        return (byte *)curr + sizeof(chunkhead);
    }
    return NULL;
}

void myfree(byte *address) {
    chunkhead *temp;
    chunkhead *curr = (chunkhead *)(address - sizeof(chunkhead));
    if(curr->next == NULL && curr->prev != NULL && ((chunkhead *)curr->prev)->info == 0) {
        temp = (chunkhead *)curr->prev;
        temp->size += curr->size;
        temp->next = NULL;
        sbrk(curr->size);
    } else{
        curr->info = 0;
        if(curr->prev != NULL && ((chunkhead *)curr->prev)->info == 0) { /*O(1)*/
            temp = (chunkhead *)curr->prev;
            temp->next = (byte *)curr->next;
            if (temp->next != NULL) { /*O(1)*/
                ((chunkhead *)temp->next)->prev = (byte *)temp;
            }
            temp->size += curr->size;
        }
        if(curr->next != NULL && ((chunkhead *)curr->next)->info == 0) { /*O(1)*/
            temp = (chunkhead *)curr->next;
            curr->next = (byte *)temp->next;
            if(curr->next != NULL) { /*O(1)*/
                ((chunkhead *)curr->next)->prev = (byte *)curr;
            }
            curr->size += temp->size;
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

chunkhead* get_last_chunk() //you can change it when you aim for performance
{
if(!head) //I have a global void *head = NULL;
return NULL;
chunkhead* ch = (chunkhead*)head;
for (; ch->next; ch = (chunkhead*)ch->next);
return ch;
}

void analyze()
{
printf("\n--------------------------------------------------------------\n");
if(!head)
{
printf("no heap, program break on address: %p\n",sbrk(0));
return;
}
chunkhead* ch = (chunkhead*)head;
for (int no=0; ch; ch = (chunkhead*)ch->next,no++)
{
printf("%d | current addr: %p |", no, ch);
printf("size: %d | ", ch->size);
printf("info: %d | ", ch->info);
printf("next: %p | ", ch->next);
printf("prev: %p", ch->prev);
printf(" \n");
}
printf("program break on address: %p\n",sbrk(0));
}