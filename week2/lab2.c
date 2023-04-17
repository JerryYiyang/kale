#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct node node;
void printMenu();
void flush();

struct node {
    node *next, *prev;
    char *data;
};

node *head = NULL;

int main(void){
    int done = 0;
    int numItems = 0;
    char choice[5];
    while(done == 0){
        printMenu();
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = '\0';
        if(strlen(choice) > 1){
            printf("invalid entry\n\n");
            continue;
        }
        if(choice[0] == '1'){
            node *new;
            node *temp;
            char *str;
            int size;
            int len;
            printf("insert text\n");
            new = malloc(sizeof(node));
            temp = head;
            size = 10;
            str = (char *)malloc(size);
            len = 0;
            while(1){
                char c;
                c = getchar();
                if (c == EOF || c == '\n') {
                    str[len] = '\0';
                    break;
                }
                str[len] = c;
                len++;
                if(len == size){
                    size = size + 10;
                    str = (char *)realloc(str, size);
                }
            }
            new->data = str;
            if(head == NULL){
                head = new;
                new->next = head;
                new->prev = head;
            } else{
                while(temp != head){
                    temp = temp->next;
                }
                new->prev = temp->prev;
                new->next = temp;
                temp->prev->next = new;
                temp->prev = new;
            }
            numItems++;
            printf("done\n\n");
        } else if(choice[0] == '2'){
            int i;
            char c[10000];
            int j;
            int check;
            check = 0;
            j = 0;
            printf("get item #\n");
            fgets(c, sizeof(c), stdin);
            while(j < sizeof(c)){
                if(c[j] == '\n'){
                    break;
                }
                if(isdigit(c[j]) == 0){
                    check = 1;
                }
                j++;
            }
            if(check == 1){
                printf("invalid entry\n\n");
                continue;
            }
            i = atoi(c);
            if(i >= 0 && i < numItems){
                node *temp = head;
                int j;
                for(j = 0; j < i; j++){
                    temp = temp->next;
                }
                printf("%s\n\n", temp->data);
            } else {
                printf("no such element\n\n");
            }
        } else if(choice[0] == '3'){
            int i;
            char c[10000];
            int j;
            int check;
            check = 0;
            j = 0;
            printf("delete item #\n");
            fgets(c, sizeof(c), stdin);
            while(j < sizeof(c)){
                if(c[j] == '\n'){
                    break;
                }
                if(isdigit(c[j]) == 0){
                    check = 1;
                }
                j++;
            }
            if(check == 1){
                printf("invalid entry\n\n");
                continue;
            }
            i = atoi(c);
            if(i >= 0 && i < numItems){
                node *temp = head;
                int j;
                for(j = 0; j < i; j++){
                    temp = temp->next;
                }
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
                if (temp == head){
                    head = temp->next;
                }
                free(temp->data);
                free(temp);
                numItems--;
                printf("done\n\n");
            } else {
                printf("no such element\n\n");
            }
        } else if(choice[0] == '4'){
            node *curr = head;
            node *temp;
            if (head != NULL){
                do {
                    temp = curr->prev;
                    curr->prev = curr->next;
                    curr->next = temp;
                    curr = curr->prev;
                } while (curr != head);
                head = temp->prev;
            }
            printf("done\n\n");
        } else if(choice[0] == '5'){
            node *temp = head;
            while(temp->next != head){
                printf("%s\n", temp->data);
                temp = temp->next;
            }
            printf("%s\n\n", temp->data);
        } else if(choice[0] == '6'){
            node *temp = head;
            node *next;
            if(head == NULL){
                done = 1;
                continue;
            }
            do {
                next = temp->next;
                free(temp->data);
                free(temp);
                temp = next;
            } while(temp != head);
            head = NULL;
            done = 1;
        } else{
            printf("invalid entry\n\n");
            continue;
        }
    }
    return 0;
}

void printMenu(){
    printf("1 push string\n2 get item\n3 delete item\n4 reverse list\n5 print list\n6 end program\n\n");
}

void flush(){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}