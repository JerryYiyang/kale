#include <stdio.h>
#include <unistd.h>


int main(){
    int fd[2], f;
    char message[30];
    pipe(fd);
    f = fork();
    if(f == 0){
        /*child writes to pipeg*/
        close(fd[0]);
        sleep(5);
        write(fd[1], "I am more important", 20);
        close(fd[1]);
    } else{
        close(fd[1]);
        /*if child has message for parent, parent should read it and print it to terminal.
        afterwards, will continue to take userinput again through stdin.*/
        for(;;){
            char message[20];
            read(STDIN_FILENO, message, 20);
            fprintf(stderr, " %s\n", message);
        }
        close(fd[0]);
        wait(0);
        return 0;
    }
}

// int main(){
//     /*use dup2() so that printf writes to out.txt and not stdout*/
//     FILE *f;
//     int fd;
//     f = fopen("out.txt", "w");
//     fd = dup(STDOUT_FILENO);
//     dup2(f->_fileno, STDOUT_FILENO);
//     printf("hello file\n");
//     fclose(f);
//     fflush(stdout);
//     dup2(fd, STDOUT_FILENO);
//     printf("hello terminal\n");
//     close(fd);
//     return 0;
// }

// int main(){
//     int fd[2], f;
//     char message[30];
//     pipe(fd);
//     f = fork();
//     if(f == 0){
//         close(fd[0]);
//         write(fd[1], "hi", 3);
//         close(fd[1]);
//     } else{
//         char message[5];
//         wait(0);
//         close(fd[1]);
//         read(fd[0], message, 5);
//         close(fd[0]);
//         fprintf(stderr, "%s\n", message);
//         return 0;
//     }
// }