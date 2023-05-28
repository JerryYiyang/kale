#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab7.h"

int main(){
char text[100];
mypipe pipeA;
init_pipe(&pipeA, 32);
mywrite(&pipeA, "hello world", 12);
mywrite(&pipeA, "it's a nice day", 16);
myread(&pipeA, text, 12);
printf("%s\n", text);
myread(&pipeA, text, 16);
printf("%s\n", text);
mywrite(&pipeA, "and now we test the carryover", 30);
myread(&pipeA, text, 30);
printf("%s\n", text);
}

void init_pipe(mypipe* pipe, int size){
    pipe->pipebuffer = malloc(size);
    pipe->start_occupied = 0;
    pipe->end_occupied = 0;
    pipe->buffersize = size;
}

int mywrite(mypipe *pipe, byte* buffer, int size){
    int i;
    if (size > (pipe->buffersize - (pipe->end_occupied - pipe->start_occupied))){
        return 0;
    }
    for(i = 0; i < size; i++){
        pipe->pipebuffer[(pipe->end_occupied + i) % pipe->buffersize] = buffer[i];
    }
    pipe->end_occupied = (pipe->end_occupied + size) % pipe->buffersize;
    return size;
}

int myread(mypipe* pipe, byte* buffer, int size){
    int i, readable;
    readable = pipe->end_occupied - pipe->start_occupied;
    if(readable < 0){
        readable += pipe->buffersize;
    }
    if(size > readable){
        size = readable;
    }
    for(i = 0; i < size; i++){
        if(pipe->start_occupied == pipe->end_occupied){
            return(i);
        }
        buffer[i] = pipe->pipebuffer[(pipe->start_occupied + i) % pipe->buffersize];
    }
    pipe->start_occupied = (pipe->start_occupied + size) % pipe->buffersize;
    return size;
}