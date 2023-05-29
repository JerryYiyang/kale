#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int *flag, *turn, fd[2], save, *wat;
/*for flag: 0 = idle, 1 = waiting, 2 = active*/
void enter(int p);
void leave(int p);
void redirect(int i);
void find_file_s(char *directory, char *filename, char *cwd, int num);
char *remove_quotes(char *input);
void find_string(char *filename, char *item, char *cwd, int num);
void find_string_s(char *directory, char *item, char *cwd, int num);

int main(){
    int f, temp, *children;
    char input[256], (*list)[10][256];
    children = mmap(NULL, 10 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    list = mmap(NULL, 10 * 256, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    flag = mmap(NULL, sizeof(int) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    turn = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    wat = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    save = dup(STDIN_FILENO);
    *wat = 10;
    pipe(fd);
    for(f = 0; f < 10; f++){
        children[f] = 0;
        strncpy((*list)[f], "", 256);
    }
    /*should while(1) be in the parent? there are some issues with printing when doing find*/
    /*there might be an issue with needing to reopen pipe. maybe need to make fd shared memory and pipe(fd) at start of while loop*/
    while(1){
        fflush(stdin);
        fprintf(stderr, ANSI_COLOR_CYAN  "findstuff$ "  ANSI_COLOR_RESET);
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        fflush(stdin);
        temp = 0;
        for(f = 0; f < 10; f++){
            if(children[f] != 0){
                temp++;
            }
        }
        if(temp == 10){
            fprintf(stderr, "There are 10 child processes running already\n");
            continue;
        }
        f = fork();
        *turn = 0;
        if(f == 0){
            DIR *dir;
            struct dirent *entry;
            char *item, *token, *flag1, *flag2;
            int num, i;
            token = strtok(input, " ");
            if(strcmp(token, "quit") == 0 || strcmp(token, "q") == 0){
                exit(0);
            } else if(strcmp(token, "kill") == 0){
                exit(0);
            } else if(strcmp(token, "list") == 0){
                exit(0);
            }
            item = strtok(NULL, " ");
            flag1 = strtok(NULL, " ");
            if(flag1 != NULL){
                flag2 = strtok(NULL, " ");
            } else{
                flag2 = NULL;
            }
            /*might have issue with timing*/
            for(i = 0; i < 10; i++){
                if(children[i] == getpid()){
                    num = i;
                    break;
                }
            }
            /*find file*/
            if(item[0] != '"'){
                if(flag1 != NULL && flag2 != NULL){
                    char *extension, *ext, cwd[1024];
                    extension = strrchr(flag1, ':');
                    extension = &extension[1];
                    getcwd(cwd, 1024);

                } else if(flag1 != NULL && flag2 == NULL){
                    if(flag1[1] == 's'){
                        char cwd[1024];
                        getcwd(cwd, 1024);
                        find_file_s(".", item, cwd, num);
                        kill(getppid(), SIGUSR1);
                        /*need something for not being able to find*/
                        exit(0);
                    } else{
                        /*i dont think f flag is necessary for finding files*/
                        char *extension, *ext;
                        extension = strrchr(flag1, ':');
                        extension = &extension[1];
                        dir = opendir(".");
                        while((entry = readdir(dir)) != NULL){
                            ext = strrchr(entry->d_name, '.');
                            if (ext != NULL) {
                                ext = &ext[1];
                            } else {
                                continue;
                            }
                            //printf("extension: %s, ext: %s\n", extension, ext);
                            if(strcmp(extension, ext) == 0 && strcmp(entry->d_name, item) == 0){
                                char cwd[1024];
                                getcwd(cwd, 1024);
                                close(fd[0]);
                                enter(num);
                                write(fd[1], &num, sizeof(int));
                                write(fd[1], cwd, 1024);
                                leave(num);
                                kill(getppid(), SIGUSR1);
                                close(fd[1]);
                                /*need something for not being able to find*/
                                exit(0);
                            }
                        }
                    }
                } else{
                    /*no flags, finding file*/
                    dir = opendir(".");
                    while((entry = readdir(dir)) != NULL){
                        if(strcmp(entry->d_name, item) == 0){
                            char cwd[1024];
                            getcwd(cwd, 1024);
                            close(fd[0]);
                            enter(num);
                            write(fd[1], &num, sizeof(int));
                            write(fd[1], cwd, 1024);
                            leave(num);
                            kill(getppid(), SIGUSR1);
                            close(fd[1]);
                            exit(0);
                        }
                    }
                    close(fd[0]);
                    enter(num);
                    write(fd[1], "file not found", strlen("file not found") + 1);
                    leave(num);
                    kill(getppid(), SIGUSR1);
                    close(fd[1]);
                    exit(0);
                }
            /*find string*/
            } else{
                struct stat path_stat;
                item = remove_quotes(item);
                if(flag1 != NULL && flag2 != NULL){
                    
                } else if(flag1 != NULL && flag2 == NULL){
                    if(flag1[1] == 's'){
                        char cwd[1024];
                        getcwd(cwd, 1024);
                        find_string_s(".", item, cwd, num);
                        kill(getppid(), SIGUSR1);
                        /*need something for not being able to find*/
                        exit(0);
                    } else{
                        char *extension, *ext, cwd[1024];
                        extension = strrchr(flag1, ':');
                        extension = &extension[1];
                        getcwd(cwd, 1024);
                        dir = opendir(".");
                        while((entry = readdir(dir)) != NULL){
                            ext = strrchr(entry->d_name, '.');
                            if (ext != NULL) {
                                ext = &ext[1];
                            } else {
                                continue;
                            }
                            if(strcmp(extension, ext) == 0){
                                find_string(entry->d_name, item, cwd, num);
                                kill(getppid(), SIGUSR1);
                                exit(0);
                            } else{
                                continue;
                            }
                        }
                    }
                } else{
                    /*no flags, finding string*/
                    char cwd[1024];
                    getcwd(cwd, 1024);
                    dir = opendir(".");
                    while((entry = readdir(dir)) != NULL){
                        stat(entry->d_name, &path_stat);
                        if (S_ISREG(path_stat.st_mode)){
                            find_string(entry->d_name, item, cwd, num);
                            kill(getppid(), SIGUSR1);
                            exit(0);
                            /*need something for not able to find*/
                        }
                    }
                }
            }
        } else{
            char *token, *copy;
            int i, num, status;
            signal(SIGUSR1, redirect);
            if(*wat != 10){
                /*running into an issue rn where this doesnt get triggered
                should be because signal() runs in parallel so wat doesnt get 
                updated in time*/
                /*also running into a seg fault here, prolly bc of the same problem*/
                waitpid(children[*wat], &status, 0);
                children[*wat] = 0;
                strncpy((*list)[*wat], "", 256);
                *wat = 10;
                continue;
            }
            copy = (char*)malloc(256 * sizeof(char));
            strncpy(copy, input, 256);
            token = strtok(input, " ");
            if(strcmp(token, "quit") == 0 || strcmp(token, "q") == 0){
                wait(0);
                for(i = 0; i < 10; i++){
                    if(children[i] != 0){
                        kill(children[i], SIGKILL);
                        //waitpid(children[i], &status, 0);
                        children[i] = 0;
                        strncpy((*list)[i], "", 256);
                    }
                }
                munmap(children, 10 * sizeof(int));
                munmap(list, 10 * 256);
                munmap(flag, sizeof(int) * 3);
                munmap(turn, sizeof(int));
                munmap(wat, sizeof(int));
                return 0;
            } else if(strcmp(token, "kill") == 0){
                int kil;
                token = strtok(NULL, " ");
                kil = atoi(token);
                wait(0);
                kill(children[kil-1], SIGKILL);
                strncpy((*list)[kil-1], "", 256);
                //waitpid(children[kil-1], &status, 0);
                continue;
            } else if(strcmp(token, "list") == 0){
                int i;
                wait(0);
                for(i = 0; i < 10; i++){
                    if(strcmp((*list)[i], "") != 0){
                        fprintf(stderr, "Child %d: %s\n", i+1, (*list)[i]);
                    }
                }
                continue;
            }
            for(i = 0; i < 10; i++){
                if(children[i] == 0){
                    children[i] = f;
                    /*this waitpid may or may not work)*/
                    waitpid(f, &status, WNOHANG);
                    strncpy((*list)[i], copy, 256);
                    break;
                }
            }
            free(copy);
        }
    }
}

void enter(int p){
/*i = 0-9 for child, 10 for parent*/
    int index;
    do{
        flag[p] = 1;
        index = *turn;
        while(index != p){
            if(flag[index] != 0){
                index = *turn;
            } else{
                index = (index + 1) % 11;
            }
        }
        flag[p] = 2;
        index = 0;
        while((index < 2) && ((index == p) || (flag[index] != 2))){
            index++;
        }
    } while(!(index >= 2) && ((*turn == p) || (flag[*turn] == 0)));
    *turn = p;
}

void leave(int p){
    int index = (*turn + 1) % 11;
    while(flag[index] == 0){
        index = (index + 1) % 11;
    }
    *turn = index;
    flag[p] = 0;
}

void redirect(int i){
    int num;
    char result[1024] = {0};
    fflush(stdin);
    dup2(fd[0], STDIN_FILENO);
    close(fd[1]);
    enter(10);
    read(STDIN_FILENO, &num, sizeof(int));
    read(STDIN_FILENO, result, 1024);
    leave(10);
    fprintf(stderr, " %s\n", result);
    dup2(save, STDIN_FILENO);
    fflush(stdin);
    *wat = num;
    close(fd[0]);
}

void find_file_s(char *directory, char *filename, char *cwd, int num){
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    char path[1024];
    char full_path[1024];
    dir = opendir(directory);
    while ((entry = readdir(dir)) != NULL){
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        stat(path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            find_file_s(path, filename, cwd, num);
        } else{
            if (strcmp(entry->d_name, filename) == 0){
                snprintf(full_path, sizeof(full_path), "%s/%s", cwd, directory);
                close(fd[0]);
                enter(num);
                write(fd[1], &num, sizeof(int));
                write(fd[1], full_path, 1024);
                leave(num);
                close(fd[1]);
            }
        }
    }
    closedir(dir);
}

/*find file with f flag is probably not necessayr*/
/*void find_file_fs(char *directory, char *filename, char *type, char *cwd, int num){
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    char path[1024], full_path[1024], *ext;
    dir = opendir(directory);
    while ((entry = readdir(dir)) != NULL){
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        stat(path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            ext = &ext[1];
            find_file_fs(path, filename, type, cwd, num);
        } else{
            ext = strrchr(entry->d_name, '.');
            if (ext != NULL) {
                ext = &ext[1];
            }else {
                continue;
            }
            if (strcmp(type, ext) == 0 && strcmp(entry->d_name, filename) == 0){
                snprintf(full_path, sizeof(full_path), "%s/%s", cwd, directory);
                close(fd[0]);
                enter(num);
                write(fd[1], &num, sizeof(int));
                write(fd[1], full_path, 1024);
                leave(num);
                close(fd[1]);
            }
        }
    }
    closedir(dir);
}*/

char *remove_quotes(char *input){
    int length = strlen(input);
    char *output = malloc(length + 1);
    int i, j = 0;
    for (i = 0; i < length; i++){
        if (input[i] != '"'){
            output[j] = input[i];
            j++;
        }
    }
    output[j] = '\0';
    return output;
}

void find_string(char *filename, char *item, char *cwd, int num){
    char buffer[1024];
    FILE *file = fopen(filename, "r");
    /*the check for file == NULL shouldn't make the program work but for some reason it works (for -s flag and its in the starting directory)*/
    if (file == NULL){
        return;
    }
    while (fgets(buffer, 1024, file)){
        if (strstr(buffer, item)){
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", cwd, filename);
            close(fd[0]);
            enter(num);
            write(fd[1], &num, sizeof(int));
            write(fd[1], full_path, 1024);
            leave(num);
            close(fd[1]);
            break;
        }
    }
    fclose(file);
}

void find_string_s(char *directory, char *item, char *cwd, int num){
    /*currently doesnt work*/
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    char path[1024];
    char full_path[1024];
    dir = opendir(directory);
    while ((entry = readdir(dir)) != NULL){
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        stat(path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            find_string_s(path, item, cwd, num);
        } else{
            snprintf(full_path, sizeof(full_path), "%s", cwd);
            find_string(entry->d_name, item, full_path, num);
        }
    }
    closedir(dir);
}