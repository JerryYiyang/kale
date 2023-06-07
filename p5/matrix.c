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
#define MATRIX_DIMENSION_XY 10

//SEARCH FOR TODO

void quadratic_matrix_multiplication_parallel(int par_id, int par_count, float *A,float *B, float *C){
    int i, row, col;
    float sum;
    for(row = 0; row < MATRIX_DIMENSION_XY; row++){
        if(row % par_count == par_id){
            for(col = 0; col < MATRIX_DIMENSION_XY; col++){
                sum = 0;
                for(i = 0; i < MATRIX_DIMENSION_XY; i++)
                {
                    sum += A[row * MATRIX_DIMENSION_XY + i] * B[i * MATRIX_DIMENSION_XY + col];
                }
                C[row * MATRIX_DIMENSION_XY + col] = sum;
            }
        }
    }
}
//************************************************************************************************************************
// sets one element of the matrix
void set_matrix_elem(float *M,int x,int y,float f)
{
M[x + y*MATRIX_DIMENSION_XY] = f;
}
//************************************************************************************************************************
// lets see it both are the same
int quadratic_matrix_compare(float *A,float *B)
{
for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
    for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
       if(A[a +b * MATRIX_DIMENSION_XY]!=B[a +b * MATRIX_DIMENSION_XY]) 
        return 0;
   
return 1;
}
//************************************************************************************************************************
//print a matrix
void quadratic_matrix_print(float *C)
{
    printf("\n");
for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
    {
    printf("\n");
    for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
        printf("%.2f,",C[a + b* MATRIX_DIMENSION_XY]);
    }
printf("\n");
}
//************************************************************************************************************************
// multiply two matrices
void quadratic_matrix_multiplication(float *A,float *B,float *C)
{
	//nullify the result matrix first
for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
    for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
        C[a + b*MATRIX_DIMENSION_XY] = 0.0;
//multiply
for(int a = 0;a<MATRIX_DIMENSION_XY;a++) // over all cols a
    for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // over all rows b
        for(int c = 0;c<MATRIX_DIMENSION_XY;c++) // over all rows/cols left
            {
                C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY] * B[a + c*MATRIX_DIMENSION_XY]; 
            }
}
//************************************************************************************************************************
void synch(int par_id,int par_count,int *ready)
{
    int i;
    ready[par_id] += 1;
    for(i = 0; i < par_count; i++){
        while(ready[par_id] > ready[i]);
    }
}
//************************************************************************************************************************
int main(int argc, char *argv[])
{
int par_id = 0; // the parallel ID of this process
int par_count = 1; // the amount of processes
float *A,*B,*C; //matrices A,B and C
int *ready; //needed for s
if(argc!=3){printf("no shared\n");}
else
    {
    par_id= atoi(argv[1]);
    par_count= atoi(argv[2]);
    }
if(par_count==1){printf("only one process\n");}

int fd[4];
if(par_id==0)
    {
    int i;
    //TODO: init the shared memory for A,B,C, ready. shm_open with C_CREAT here! then ftruncate! then mmap
    fd[0] = shm_open("Am", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[1] = shm_open("Bm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[2] = shm_open("Cm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[3] = shm_open("s", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    for (i = 0; i < 3; i++){
        ftruncate(fd[i], sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY);
    }
    ftruncate(fd[3], sizeof(int) * par_count);
    A = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
    B = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
    C = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
    ready = mmap(NULL, sizeof(int) * par_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
    for(i = 0; i < par_count; i++){
        ready[i] = 0;
    }
    }
else
    {
    sleep(2);
	//TODO: init the shared memory for A,B,C, ready. shm_open withOUT C_CREAT here! NO ftruncate! but yes to mmap
    fd[0] = shm_open("Am", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[1] = shm_open("Bm", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[2] = shm_open("Cm", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    fd[3] = shm_open("s", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    A = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
    B = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
    C = mmap(NULL, sizeof(float) * MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
    ready = mmap(NULL, sizeof(int) * par_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
    }

synch(par_id,par_count,ready);

if(par_id==0)
    {
	//TODO: initialize the matrices A and B
        int i, j;
        for(i = 0; i < MATRIX_DIMENSION_XY; i++){
            for(j = 0; j < MATRIX_DIMENSION_XY; j++){
                set_matrix_elem(A, i, j, (float)i);
                set_matrix_elem(B, i, j, (float)j);
            }
        }
    }

synch(par_id,par_count,ready);

quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C);

synch(par_id,par_count,ready);

if(par_id==0)
    quadratic_matrix_print(C);
synch(par_id, par_count, ready);


//lets test the result:
float M[MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY];
quadratic_matrix_multiplication(A, B, M);
if (quadratic_matrix_compare(C, M))
	printf("full points!\n");
else
	printf("buuug!\n");


close(fd[0]);
close(fd[1]);
close(fd[2]);
close(fd[3]);
shm_unlink("Am");
shm_unlink("Bm");
shm_unlink("Cm");
shm_unlink("s");

return 0;    
}