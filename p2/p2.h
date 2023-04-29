#include <stdio.h>
#include <stdlib.h>

typedef struct chunkhead
{
    unsigned int size;
    unsigned int info; /*0 if it is free, 1 if it is not free*/
    unsigned char *next,*prev;
}chunkhead;

unsigned char *mymalloc(unsigned int size);
void myfree(unsigned char *address);
void analyse();