#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef struct chunkhead
{
    unsigned int size;
    unsigned int info; /*0 if it is free, 1 if it is not free*/
    unsigned char *next,*prev;
}chunkhead;

typedef struct freechunk
{
    unsigned int size;
    unsigned char *next, *address;
}freechunk;

unsigned char *mymalloc(unsigned int size);
void myfree(unsigned char *address);
void analyse();