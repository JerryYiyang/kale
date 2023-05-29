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

void enter(int p);
void leave(int p);
void redirect(int i);
void find_file_s(char *directory, char *filename, char *cwd, int num);
char *remove_quotes(char *input);
void find_string(char *filename, char *item, char *cwd, int num);
void find_string_s(char *directory, char *item, char *cwd, int num);
void find_string_fs(char *directory, char *item, char *type, char *cwd, int num);