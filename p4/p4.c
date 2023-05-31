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
#include "p4.h"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int *flag, *turn, fd[2], save, *children, found, *stfound;
/*for flag: 0 = idle, 1 = waiting, 2 = active*/
char(*list)[10][256];

int main(){
    int f, temp;
    ssize_t len;
    char input[1024], result[1024];
    children = mmap(NULL, 10 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    list = mmap(NULL, 10 * 256, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    flag = mmap(NULL, sizeof(int) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    turn = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    stfound = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    save = dup(STDIN_FILENO);
    for(f = 0; f < 10; f++){
        children[f] = 0;
        strncpy((*list)[f], "", 256);
    }
    *turn = 0;
    pipe(fd);
    while(1){
        found = 0;
        fflush(stdin);
        fprintf(stderr, ANSI_COLOR_CYAN  "findstuff$ "  ANSI_COLOR_RESET);
        len = read(STDIN_FILENO, input, 1024);
        if(len > 0){
            input[len] = '\0';
            if(input[0] == '/'){
                fprintf(stderr, "%s\n", input);
                dup2(save, STDIN_FILENO);
            }
        } else{
            read(STDIN_FILENO, input, 256);
        }
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
        if(f == 0){
            DIR *dir;
            struct dirent *entry;
            char *item, *token, *flag1, *flag2;
            int num, i;
            close(fd[0]);
            for(i = 0; i < 10; i++){
                if(children[i] == 0){
                    children[i] = f;
                    strncpy((*list)[i], input, 256);
                    num = i;
                    break;
                }
            }
            token = strtok(input, " ");
            if(strcmp(token, "quit") == 0 || strcmp(token, "q") == 0){
                close(fd[1]);
                exit(0);
            } else if(strcmp(token, "kill") == 0){
                close(fd[1]);
                exit(0);
            } else if(strcmp(token, "list") == 0){
                close(fd[1]);
                exit(0);
            }
            item = strtok(NULL, " ");
            if (item[0] == '"' && item[strlen(item)-1] != '"'){
                /*will have to free item_buffer but i'm not sure when/where*/
                char *next_token;
                char *item_buffer = malloc(strlen(input) + 1);
                strcpy(item_buffer, item);
                do{
                    next_token = strtok(NULL, " ");
                    strcat(item_buffer, " ");
                    strcat(item_buffer, next_token);
                } while(next_token[strlen(next_token)-1] != '"');
                item = item_buffer;
            }
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
                if(flag1 != NULL && flag2 == NULL){
                    if(flag1[1] == 's'){
                        char cwd[1024];
                        getcwd(cwd, 1024);
                        find_file_s(".", item, cwd, num);
                        if(found == 0){
                            enter(num);
                            write(fd[1], "file not found", strlen("file not found") + 1);
                            leave(num);
                        }
                        close(fd[1]);
                        kill(getppid(), SIGUSR1);
                        exit(0);
                    }
                } else{
                    /*no flags, finding file*/
                    dir = opendir(".");
                    while((entry = readdir(dir)) != NULL){
                        if(strcmp(entry->d_name, item) == 0){
                            char cwd[1024];
                            getcwd(cwd, 1024);
                            enter(num);
                            write(fd[1], cwd, 1024);
                            leave(num);
                            close(fd[1]);
                            kill(getppid(), SIGUSR1);
                            exit(0);
                        }
                    }
                    enter(num);
                    write(fd[1], "file not found", strlen("file not found") + 1);
                    leave(num);
                    close(fd[1]);
                    kill(getppid(), SIGUSR1);
                    exit(0);
                }
            /*find string*/
            } else{
                struct stat path_stat;
                item = remove_quotes(item);
                if(flag1 != NULL && flag2 != NULL){
                    char *extension, *ext, cwd[1024];
                    extension = strrchr(flag1, ':');
                    extension = &extension[1];
                    getcwd(cwd, 1024);
                    find_string_fs(".", item, extension, cwd, num);
                    if(found == 0){
                        enter(num);
                        write(fd[1], "string not found", strlen("string not found") + 1);
                        leave(num);
                    }
                    close(fd[1]);
                    kill(getppid(), SIGUSR1);
                    exit(0);
                } else if(flag1 != NULL && flag2 == NULL){
                    if(flag1[1] == 's'){
                        char cwd[1024];
                        getcwd(cwd, 1024);
                        find_string_s(".", item, cwd, num);
                        if(found == 0){
                            enter(num);
                            write(fd[1], "string not found", strlen("string not found") + 1);
                            leave(num);
                        }
                        close(fd[1]);
                        kill(getppid(), SIGUSR1);
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
                                if(found == 1){
                                    close(fd[1]);
                                    kill(getppid(), SIGUSR1);
                                    exit(0);
                                }
                            } else{
                                continue;
                            }
                        }
                        enter(num);
                        write(fd[1], "string not found", strlen("string not found") + 1);
                        leave(num);
                        close(fd[1]);
                        kill(getppid(), SIGUSR1);
                        exit(0);
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
                            if(found == 0){
                                enter(num);
                                write(fd[1], "string not found", strlen("string not found") + 1);
                                leave(num);
                            }
                            close(fd[1]);
                            kill(getppid(), SIGUSR1);
                            exit(0);
                        }
                    }
                }
            }
        } else{
            char *token, *copy;
            int i, num, status;
            close(fd[1]);
            signal(SIGUSR1, redirect);
            /*i think i should move this into child, but for some reason when i did the program stopped working*/
            copy = (char*)malloc(256 * sizeof(char));
            // strncpy(copy, input, 256);
            // for(i = 0; i < 10; i++){
            //     if(children[i] == 0){
            //         children[i] = f;
            //         strncpy((*list)[i], copy, 256);
            //         break;
            //     }
            // }
            token = strtok(input, " ");
            if(strcmp(token, "quit") == 0 || strcmp(token, "q") == 0){
                for(i = 0; i < 10; i++){
                    if(children[i] != 0){
                        kill(children[i], SIGKILL);
                        waitpid(children[i], NULL, 0);
                    }
                }
                munmap(children, 10 * sizeof(int));
                munmap(list, 10 * 256);
                munmap(flag, sizeof(int) * 3);
                munmap(turn, sizeof(int));
                free(copy);
                close(fd[0]);
                return 0;
            } else if(strcmp(token, "kill") == 0){
                int kil;
                token = strtok(NULL, " ");
                kil = atoi(token);
                kill(children[kil-1], SIGKILL);
                //waitpid(children[kil-1], NULL, 0);
                children[kil-1] = 0;
                strncpy((*list)[kil-1], "", 256);
                wait(0);
                for(kil = 0; kil < 10; kil++){
                    if(children[kil] == f){
                        children[kil] = 0;
                        strncpy((*list)[kil], "", 256);
                    }
                }
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
            // for(i = 0; i < 10; i++){
            //     if(children[i] != 0){
            //         int t;
            //         t = waitpid(children[i], NULL, WNOHANG);
            //         if(t == 0){
            //             children[i] = 0;
            //             strncpy((*list)[i], "", 256);
            //         }
            //     }
            // }
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
        while((index < 11) && ((index == p) || (flag[index] != 2))){
            index++;
        }
    } while(!(index >= 11) && ((*turn == p) || (flag[*turn] == 0)));
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
    /*maybe reset stfound here*/
    dup2(fd[0], STDIN_FILENO);
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
        } else if(S_ISREG(path_stat.st_mode)){
            if (strcmp(entry->d_name, filename) == 0){
                snprintf(full_path, sizeof(full_path), "%s/%s", cwd, directory);
                //sleep(5);
                close(fd[0]);
                enter(num);
                write(fd[1], full_path, 1024);
                leave(num);
                close(fd[1]);
                found = 1;
            }
        }
    }
    closedir(dir);
}

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

void find_string(char *filepath, char *item, char *cwd, int num){
    char buffer[1024], full_path[1024];
    FILE *file = fopen(filepath, "r");
    if (file == NULL){
        return;
    }
    while (fgets(buffer, 1024, file)){
        if (strstr(buffer, item)){
            /*have a global var with shared mem that increments by 1 every time the string gets found*/
            snprintf(full_path, sizeof(full_path), "%s/%s", cwd, filepath);
            enter(num);
            write(fd[1], full_path, 1024);
            leave(num);
            found = 1;
            stfound++;
            break;
        }
    }
    fclose(file);
}

/*currently can't write multiple directories into pipe. tested it so i know it does know the number of directories the string appears in*/
void find_string_s(char *directory, char *item, char *cwd, int num){
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    char path[1024], full_path[1024];
    dir = opendir(directory);
    while ((entry = readdir(dir)) != NULL){
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        stat(path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            find_string_s(path, item, cwd, num); 
        } else if(S_ISREG(path_stat.st_mode)){
            snprintf(full_path, sizeof(full_path), "%s", cwd);
            find_string(path, item, cwd, num); 
        }
    }
    closedir(dir);
}

void find_string_fs(char *directory, char *item, char *type, char *cwd, int num){
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
            find_string_fs(path, item, type, cwd, num);
        } else if(S_ISREG(path_stat.st_mode)){
            ext = strrchr(entry->d_name, '.');
            if (ext != NULL){
                ext = &ext[1];
            } else{
                continue;
            }
            if (strcmp(type, ext) == 0){
                snprintf(full_path, sizeof(full_path), "%s", cwd);
                find_string(path, item, cwd, num); 
            }
        }
    }
    closedir(dir);
}