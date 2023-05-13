#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "main.h"

int main(int argc, char *argv[]){
    FILE *img, *out;
    char *flag, *header, **code, *comp;
    BYTE *pixel, *grey;
    int width, height;
    int pcount, padding;
    color *c; /*have to chanve to color *c for non grey*/
    int x, y, i, n, h, pix, *freq, stlen;
    BYTE null;
    BYTE *N;
    FILEHEADER *fileHeader;
    INFOHEADER *infoHeader;
    flag = argv[1];
    stlen = strlen(argv[2]);
    comp = &argv[2][stlen-4];
    if(comp[0] == '.'){
        img = fopen(argv[2], "rb");
        fileHeader = mmap(NULL, sizeof(FILEHEADER), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
        infoHeader = mmap(NULL, sizeof(INFOHEADER), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
        readFHeader(fileHeader, img);
        readIHeader(infoHeader, img);
        width = infoHeader->biWidth;
        height = infoHeader->biHeight;
        pcount = width * height;
        padding = (4 - (width * 3) % 4) % 4;
        fseek(img, fileHeader->bfOffBits, SEEK_SET);
        pixel = (BYTE *)mmap(NULL, pcount * 3 + height * padding, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
        fread(pixel, 1, pcount * 3 + height * padding, img);
        c = mmap(NULL, pcount * sizeof(color), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
        grey = mmap(NULL, pcount, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
        for(y = 0; y < height; y++){
            for(x = 0; x < width; x++){
                c[y * width + x] = get_color(pixel, width, height, x, y, padding);
            }
        }
        n = fork();
        h = (int)(height / 2);
        if(height % 2 != 0){
            h += 1;
        }
        pix = width * h;
        if(n == 0){
            for(i = 0; i < pix; i++){
                grey[i] = make_grey((color)c[i]);
            }
        } else{
            for(i = pix; i < pcount; i++){
                grey[i] = make_grey((color)c[i]);
            }
            wait(0);
            freq = cnt_freq(grey, pcount);
            header = create_header(freq);
            code = encode(freq);


            out = fopen("out.bmp", "wb");
            writeFHeader(fileHeader, out);
            writeIHeader(infoHeader, out);
            null = '\0';
            N = &null;
            for(i = 0; i < pcount; i++){
                int avg = (int)grey[i];
                printf
                c[i].blue = (BYTE)avg;
                c[i].green = (BYTE)avg;
                c[i].red = (BYTE)avg;
                fwrite(&c[i], sizeof(color), 1, out);
                if(padding != 0 && (i + 1) % width == 0){
                    fwrite(N, sizeof(BYTE), padding, out);
                }
            }
            fclose(img);
            fclose(out);
            munmap(fileHeader, sizeof(FILEHEADER));
            munmap(infoHeader, sizeof(INFOHEADER));
            munmap(pixel, pcount * 3 + height * padding);
            munmap(c, pcount * sizeof(color));
            munmap(grey, pcount);

            free(freq);
            free(header);
            free(code);
            }
    } else{
        return 0;
    }
}

void readFHeader(FILEHEADER *fh, FILE *f){
    fread(&(fh->bfType), sizeof(WORD), 1, f);
    fread(&(fh->bfSize), sizeof(DWORD), 1, f);
    fread(&(fh->bfReserved1), sizeof(WORD), 1, f);
    fread(&(fh->bfReserved2), sizeof(WORD), 1, f);
    fread(&(fh->bfOffBits), sizeof(DWORD), 1, f);
}

void readIHeader(INFOHEADER *ih, FILE *f){
    fread(&(ih->biSize), sizeof(DWORD), 1, f);
    fread(&(ih->biWidth), sizeof(LONG), 1, f);
    fread(&(ih->biHeight), sizeof(LONG), 1, f);
    fread(&(ih->biPlanes), sizeof(WORD), 1, f);
    fread(&(ih->biBitCount), sizeof(WORD), 1, f);
    fread(&(ih->biCompression), sizeof(DWORD), 1, f);
    fread(&(ih->biSizeImage), sizeof(DWORD), 1, f);
    fread(&(ih->biXPelsPerMeter), sizeof(LONG), 1, f);
    fread(&(ih->biYPelsPerMeter), sizeof(LONG), 1, f);
    fread(&(ih->biClrUsed), sizeof(DWORD), 1, f);
    fread(&(ih->biClrImportant), sizeof(DWORD), 1, f);
}

void writeFHeader(FILEHEADER *fh, FILE *f){
    fwrite(&(fh->bfType), sizeof(WORD), 1, f);
    fwrite(&(fh->bfSize), sizeof(DWORD), 1, f);
    fwrite(&(fh->bfReserved1), sizeof(WORD), 1, f);
    fwrite(&(fh->bfReserved2), sizeof(WORD), 1, f);
    fwrite(&(fh->bfOffBits), sizeof(DWORD), 1, f);
}

void writeIHeader(INFOHEADER *ih, FILE *f){
    fwrite(&(ih->biSize), sizeof(DWORD), 1, f);
    fwrite(&(ih->biWidth), sizeof(LONG), 1, f);
    fwrite(&(ih->biHeight), sizeof(LONG), 1, f);
    fwrite(&(ih->biPlanes), sizeof(WORD), 1, f);
    fwrite(&(ih->biBitCount), sizeof(WORD), 1, f);
    fwrite(&(ih->biCompression), sizeof(DWORD), 1, f);
    fwrite(&(ih->biSizeImage), sizeof(DWORD), 1, f);
    fwrite(&(ih->biXPelsPerMeter), sizeof(LONG), 1, f);
    fwrite(&(ih->biYPelsPerMeter), sizeof(LONG), 1, f);
    fwrite(&(ih->biClrUsed), sizeof(DWORD), 1, f);
    fwrite(&(ih->biClrImportant), sizeof(DWORD), 1, f);
}

color get_color(BYTE *pixel, int width, int height, int x, int y, int padding){
    color c;
    int i;
    if (x < 0 || x >= width || y < 0 || y >= height) {
        exit(1);
    }
    i = (y * (width * 3 + padding) + x * 3);
    c.blue = pixel[i];
    c.green = pixel[i + 1];
    c.red = pixel[i + 2];
    return c;
}

BYTE make_grey(color c){
    int avg;
    avg = ((int)c.blue + (int)c.green + (int)c.red) / 3;
    return (BYTE)avg;
}

/*huffman stuff*/
int *cnt_freq(BYTE *c, int pcount){
    int *freq = (int *)calloc(256, sizeof(int));
    int i;
    for (i = 0; i < pcount; i++) {
        // For grey images, since BGR items are the same, only need 1 (but will need to change for color)
        freq[(int)c[i]] += 1;
    }
    return freq;
}

void free_huff_tree(huff* node) {
    if (node == NULL) {
        return;
    }

    free_huff_tree(node->left);
    free_huff_tree(node->right);
    free(node);
}


char *create_header(int *freq){
    char *result = (char *)malloc(256 * 10 * sizeof(char));
    char buffer[20];
    result[0] = '\0';
    for(int i = 0; i < 256; i++){
        if(freq[i] != 0){
            sprintf(buffer, "%d %d ", i, freq[i]);
            strcat(result, buffer);
        }
    }
    int len = strlen(result);
    if(len > 0){
        result[len - 1] = '\0';
    }
    return result;
}

int *parse_header(const char *header_string){
    int *freqlist = (int *)calloc(256, sizeof(int));
    int asc, freq;
    const char *p = header_string;
    const char *next_space;

    while (sscanf(p, "%d %d", &asc, &freq) == 2) {
        freqlist[asc] = freq;
        next_space = strchr(p, ' ');
        if (next_space == NULL) {
            break;
        }
        p = next_space + 1;
        next_space = strchr(p, ' ');
        if (next_space == NULL) {
            break;
        }
        p = next_space + 1;
    }
    return freqlist;
}

char **encode(int *freqlist){
    huff *root;
    char **codes;
    root = create_huff_tree(freqlist);
    codes = create_code(root);
    for (int i = 0; i < 256; ++i) {
        if (codes[i] != NULL) {
            printf("Character ASCII: %d, Huffman code: %s\n", i, codes[i]);
        }
    }
    free_huff_tree(root);
}

huff *create_huff_tree(int *freqlist){
    int i;
    huff *x, *y;
    orderedList *ol = create_ordered_list();
    for(i = 0; i < 256; i++){
        if(freqlist[i] != 0){
            huff *node = malloc(sizeof(huff));
            node->left = NULL;
            node->right = NULL;
            node->data = i;
            node->freq = freqlist[i];
            insert(ol, node);
        }
    }
    while(ol->size > 1){
        huff *top = malloc(sizeof(huff));
        x = pop(ol);
        y = pop(ol);
        i = x->data + y->data;
        if (x->data < y->data){
            top->data = x->data;
        } else if(y->data < x->data){
            top->data = y->data;
        }
        top->freq = i;
        top->left = x;
        top->right = y;
        insert(ol, top);
    }
    x = pop(ol);
    free(ol);
    return x;
}

void code_help(huff* node, char** lst, char* path){
    if (is_leaf(node)) {
        lst[node->data] = strdup(path);
    } else {
        char* left_path = malloc(strlen(path) + 2);
        strcpy(left_path, path);
        strcat(left_path, "0");
        code_help(node->left, lst, left_path);
        free(left_path);

        char* right_path = malloc(strlen(path) + 2);
        strcpy(right_path, path);
        strcat(right_path, "1");
        code_help(node->right, lst, right_path);
        free(right_path);
    }
}

char** create_code(huff* node){
    char** lst = (char**)calloc(256, sizeof(char*));
    if (node != NULL) {
        code_help(node, lst, "");
    }
    return lst;
}

int is_leaf(huff* root){
    if(root->left == NULL && root->right == NULL){
        return 1;
    }
}

void insert(orderedList *ol, huff *item){
    Node *head = ol->head;
    Node *new = (Node *)malloc(sizeof(Node));
    new->item = item;
    ol->size++;
    printf("item freq: %d", item->freq);
    if (head->next == NULL) {
        head->next = new;
        new->prev = head;
        head->prev = new;
        new->next = head;
        return;
    }
    if (item->freq < head->next->item->freq) {
        new->prev = head;
        new->next = head->next;
        head->next->prev = new;
        head->next = new;
        return;
    }
    Node *temp = head->next;
    while (temp->item->freq < item->freq) {
        //printf("temp item freq:%d  item freq:%d\n", temp->item->freq, item->freq);
        temp = temp->next;
    }
    new->prev = temp->prev;
    new->next = temp;
    temp->prev->next = new;
    temp->prev = new;
}

huff *pop(orderedList* ol){
    Node *node;
    huff *item;
    node = ol->head->next;
    ol->head->next = node->next;
    node->next->prev = ol->head;
    item = node->item;
    ol->size--;
    free(node);
    return item;
}

orderedList *create_ordered_list(){
    orderedList *ol = (orderedList *)malloc(sizeof(orderedList));
    Node *dummy = (Node *)malloc(sizeof(Node));
    dummy->prev = dummy;
    dummy->next = NULL;
    dummy->item = NULL;
    ol->head = dummy;
    ol->size = 0;
    return ol;
}