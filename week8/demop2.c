#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

/*p2*/


int main()
{
/*opens a shared memory with a name */   
int fd = shm_open("CSC357", O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
/*mmap*/
int *p = mmap(NULL, 10 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


fprintf(stderr, "p[0] =  %d \n", p[0]);


/*clean up*/
close(fd);
shm_unlink("CSC357");
munmap(p, 10 * sizeof(int) );
return 0;
}