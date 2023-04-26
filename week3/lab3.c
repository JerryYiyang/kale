#include <stdio.h>
#include <stdlib.h>
#include "lab3.h"
#define PAGESIZE 1024

unsigned char myheap[1048576];

chunkhead *head = NULL;


int main(void){
    return 0;
}

unsigned char *mymalloc(unsigned int size) {
    chunkhead *open;
    if (size % PAGESIZE != 0) {
        size = ((int)(size / PAGESIZE) + 1) * PAGESIZE;
    }
    if (head == NULL) {
        head = (chunkhead *)myheap;
        head->info = 1;
        head->prev = NULL;
        head->size = size;
        open = (chunkhead *)((unsigned char *)head + size + sizeof(chunkhead));
        open->next = NULL;
        open->prev = (unsigned char *)head;
        open->info = 0;
        open->size = 1048576 - size - sizeof(chunkhead);
        head->next = (unsigned char *)open;
        return (unsigned char *)head + sizeof(chunkhead);
    } else {
        chunkhead *curr = (chunkhead *)head;
        while (curr != NULL) {
            if (curr->info == 0 && curr->size >= size) {
                int remainder = curr->size - size - sizeof(chunkhead);
                curr->info = 1;
                curr->size = size;
                open = (chunkhead *)((unsigned char *)curr + size + sizeof(chunkhead));
                open->info = 0;
                open->size = remainder;
                open->next = curr->next;
                open->prev = (unsigned char *)curr;
                curr->next = (unsigned char *)open;
                if (open->next != NULL) {
                    ((chunkhead *)open->next)->prev = (unsigned char *)open;
                }
                return (unsigned char *)curr + sizeof(chunkhead);
            }
            curr = (chunkhead *)curr->next;
        }
    }
    return NULL;
}


void myfree(unsigned char *address) {
    chunkhead *curr = (chunkhead *)(address - sizeof(chunkhead));
    chunkhead *temp;
    curr->info = 0;
    if (curr->prev != NULL && ((chunkhead *)curr->prev)->info == 0) {
        temp = (chunkhead *)curr->prev;
        temp->next = (unsigned char *)curr->next;
        if (temp->next != NULL){
            ((chunkhead *)temp->next)->prev = (unsigned char *)temp;
        }
        temp->size += curr->size + sizeof(chunkhead);
    }
    if (curr->next != NULL && ((chunkhead *)curr->next)->info == 0) {
        temp = (chunkhead *)curr->next;
        curr->next = (unsigned char *)temp->next;
        if (curr->next != NULL){
            ((chunkhead *)curr->next)->prev = (unsigned char *)curr;
        }
        curr->size += temp->size + sizeof(chunkhead);
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
