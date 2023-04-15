#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100000

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
    char choice;
    while(done == 0){
        printMenu();
        choice = getchar();
        flush();
        if(choice == '1'){
            node *new;
            node *temp;
            char *str;
            int len;
            printf("insert text\n");
            new = malloc(sizeof(node));
            temp = head;
            str = (char *) malloc(SIZE);
            fgets(str, SIZE, stdin);
            len = strlen(str);
            new->data = malloc(len);
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
        } else if(choice == '2'){
            int i,c;
            printf("get item #\n");
            c = getchar();
            flush();
            i = c - '0';
            if(i >= 0 && i < numItems){
                node *temp = head;
                int j;
                for(j = 0; j < i; j++){
                    temp = temp->next;
                }
                printf("%s\n", temp->data);
            } else {
                printf("no such element\n\n");
            }
        } else if(choice == '3'){
            int i,c;
            printf("delete item #\n");
            c = getchar();
            flush();
            i = c - '0';
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
        } else if(choice == '4'){
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
        } else if(choice == '5'){
            node *temp = head;
            while(temp->next != head){
                printf("%s", temp->data);
                temp = temp->next;
            }
            printf("%s\n", temp->data);
        } else if(choice == '6'){
            node *temp = head;
            node *next;
            do {
                next = temp->next;
                free(temp->data);
                free(temp);
                temp = next;
            } while(temp != head);
            head = NULL;
            done = 1;
        } else{
            printf("invalid input\n\n");
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