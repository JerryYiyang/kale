#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef struct mypipe
{
byte* pipebuffer;
int buffersize;
int start_occupied;
int end_occupied;
}mypipe;

//initializes (malloc) the pipe with a size of "size" and sets start and end.
void init_pipe(mypipe* pipe, int size);
//writes "size" bytes from buffer into the pipe, returns size
int mywrite(mypipe *pipe, byte* buffer, int size);
//reads "size" bytes from pipe into buffer, returns how much it read (max size), 0 if pipe is empty
int myread(mypipe* pipe, byte* buffer, int size);