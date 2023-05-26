#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>


/*p1*/
int main()
{
/*opens a shared memory with a name */   
int fd = shm_open("CSC357",O_CREAT | O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
/*sets the size of the named shared memory*/
ftruncate(fd, 10 * sizeof(int));
/*mmap*/
int *p = mmap(NULL, 10 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


p[0] = 5;


/*start program 2*/
int pid = fork();
if(pid == 0){
    execv("./demop2", NULL);
} else{
    wait(0);
}

/*clean up*/
close(fd);
shm_unlink("CSC357");
munmap(p, 10 * sizeof(int) );
return 0;
}





// /*p1*/
// int main()
// {
//     int save, fd[2];
//     fprintf(stderr,"I am program demo with pid = %d \n", getpid());
//     char* text = "Hello \n how \n are\n you\n";
//     save = dup(STDIN_FILENO);
//     pipe(fd);
//     int pid = fork();
//     if(pid == 0){
//         /*starts sort*/
//         close(fd[1]);
//         dup2(fd[0], STDIN_FILENO);
//         close(fd[0]);
//         char *argv[] = {(char*)"sort", NULL};
//         execvp(argv[0],argv);
//         return 0;
//     }
//     else{
//         close(fd[0]);
//         write(fd[1], text, 24);
//         close(fd[1]);
//         wait(NULL);   
//         fprintf(stderr,"Progam demo after sorting \n");

//     }
//     return 0;
// }
