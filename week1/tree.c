#include <stdio.h>
#include <string.h>
#include <ctype.h>

int atoi(const char *str);
int getInput();

int main(int argc, char *argv[]){
    int height = 0;
    if(argc == 1){
        height = getInput();
    } else if(argc == 2){
        if(atoi(argv[1]) == 0){
            printf("The input must be an integer between 4 and 15 inclusive.\n");
            height = getInput();
        } else {
            height = atoi(argv[1]);
        }
    } else{
        if(atoi(argv[1]) == 0){
            printf("The input must be an integer between 4 and 15 inclusive.\n");
            height = getInput();
        } else {
            height = atoi(argv[1]);
        }
        FILE *fl;
        fl = fopen(argv[2], "w");
        int width = (height - 3) * 2 - 2;
        char leaf[30] = "*";
        for(int i = 0; i < height - 3; i++){
            for(int j = 0; j < width; j++){
                fprintf(fl, " ");
            }
            fprintf(fl, "%s", leaf);
            fprintf(fl, "\n");
            width -= 1;
            char temp = '*';
            strncat(leaf, &temp, 1);
            strncat(leaf, &temp, 1);
        }
        width = (height - 3) * 2 - 2;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < width; j++){
                fprintf(fl, " ");
            }
            fprintf(fl, "*");
            fprintf(fl, "\n");
        }
        fclose(fl);
    }
    int width = (height - 3) * 2 - 2;
    char leaf[30] = "*";
    for(int i = 0; i < height - 3; i++){
        for(int j = 0; j < width; j++){
            printf(" ");
        }
        printf("%s", leaf);
        printf("\n");
        width -= 1;
        char temp = '*';
        strncat(leaf, &temp, 1);
        strncat(leaf, &temp, 1);
    }
    width = (height - 3) * 2 - 2;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < width; j++){
            printf(" ");
        }
        printf("*");
        printf("\n");
    }
    return 0;
}


int getInput(){
        int height = 0;
        while((height < 3 || height > 15)){
        printf("What is the height of the tree?\n");
        if(scanf("%d", &height) == 0){
            char trash[10000];
            scanf("%s", &trash);
            printf("The input must be an integer.\n");
            continue;
        } else{
            if(height < 3 || height > 15){
                printf("The integer must be between 4 and 15 inclusive.\n");
            }
        }
    }
    return(height);
}