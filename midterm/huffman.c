#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "huffman.h"
#include "main.h"
#include "ordered_list.h"
#define true 0
#define false 1

int *cnt_freq(BYTE *c, int pcount){
    int *freq = (int *)calloc(256, sizeof(int));
    int i;
    for (i = 0; i <= pcount; i++) {
        // For grey images, since BGR values are the same, only need 1 (but will need to change for color)
        freq[(int)c[i]] += 1;
    }
    return freq;
}


void code_help(huff *node, char **lst, char *path){
    if(is_leaf(node)){
        lst[node->c] = strdup(path);
    } else{
        char left_path[256];
        strcpy(left_path, path);
        strcat(left_path, "0");
        code_help(node->left, lst, left_path);

        char right_path[256];
        strcpy(right_path, path);
        strcat(right_path, "1");
        code_help(node->right, lst, right_path);
    }
}

char **create_code(huff *node){
    char **lst = (char **)calloc(256, sizeof(char *));
    if (node != NULL){
        code_help(node, lst, "");
    }
    return lst;
}

void free_code(char **lst){
    for(int i = 0; i < 256; i++){
        if(lst[i] != NULL){
            free(lst[i]);
        }
    }
    free(lst);
}

int is_leaf(huff *node){
    if(node->left == NULL && node->right == NULL){
        return true;
    }
    return false;
}

/*might have to make a free function for this, not sure since the node from ordered list gets freed during pop. might still have to free huff node*/
huff *create_huff_tree(int *freq){
    orderedList *ordered = create_ordered_list();
    int i, f;
    huff *temp, *top, *x, *y;
    for(i = 0; i < 256; i++){
        if(freq[i] != 0){
            temp->c = i;
            temp->freq = freq[i];
            insert(ordered, temp);
        }
    }
    if(size(ordered) == 0){
        return NULL;
    }
    while(size(ordered) > 1){
        x = pop(ordered, 0);
        y = pop(ordered, 0);
        f = x->freq + y->freq;
        if(x->c < y->c){
            top->c = x->c;
        } else{
            top->c = y->c;
        }
        top->freq = f;
        top->left = x;
        top->right = y;
        insert(ordered, top);
    }
    return pop(ordered, 0);
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

    while (sscanf(p, "%d %d", &asc, &freq) == 2) {
        freqlist[asc] = freq;
        p = strchr(p, ' ') + 1;
        p = strchr(p, ' ') + 1;
    }

    return freqlist;
}

char *encode_help(int *freqlist, BYTE *c, int pcount){
    huff *hufftree = create_huff_tree(freqlist);
    char **codes = create_code(hufftree);
    char *code = (char *)calloc(1, sizeof(char));
    int i;
    for(i = 0; i < pcount; i++){
        char *new_code = (char *)calloc(strlen(code) + strlen(codes[c[i]]) + 1, sizeof(char));
        strcpy(new_code, code);
        strcat(new_code, codes[(int)c[i]]);
        free(code);
        code = new_code;
    }
    free_code(codes);
    return code;
}

HuffmanBitWriter *huffman_bit_writer_init(const char *fname){
    HuffmanBitWriter *writer = (HuffmanBitWriter *)malloc(sizeof(HuffmanBitWriter));
    writer->file = fopen(fname, "wb");
    writer->n_bits = 0;
    writer->byte = 0;
    return writer;
}

void huffman_bit_writer_close(HuffmanBitWriter *writer){
    if (writer->n_bits > 0) {
        writer->byte <<= (7 - writer->n_bits);
        fwrite(&(writer->byte), sizeof(uint8_t), 1, writer->file);
    }
    fclose(writer->file);
    free(writer);
}

void huffman_bit_writer_write_str(HuffmanBitWriter *writer, const char *str){
    fwrite(str, sizeof(char), strlen(str), writer->file);
}

void huffman_bit_writer_write_code(HuffmanBitWriter *writer, const char *code){
    for (int i = 0; i < strlen(code); i++) {
        char bit = code[i];
        if (bit == '1') {
            writer->byte += 1;
        }
        if (writer->n_bits == 7) {
            fwrite(&(writer->byte), sizeof(uint8_t), 1, writer->file);
            writer->byte = 0;
            writer->n_bits = 0;
        } else {
            writer->byte <<= 1;
            writer->n_bits += 1;
        }
    }
}

HuffmanBitReader *huffman_bit_reader_init(const char *fname){
    HuffmanBitReader *reader = (HuffmanBitReader *)malloc(sizeof(HuffmanBitReader));
    reader->file = fopen(fname, "rb");
    reader->n_bits = 0;
    reader->byte = 0;
    reader->mask = 0;
    return reader;
}

void huffman_bit_reader_close(HuffmanBitReader *reader){
    fclose(reader->file);
    free(reader);
}

char *huffman_bit_reader_read_str(HuffmanBitReader *reader){
    char *buffer = NULL;
    size_t buffer_size = 0;
    getline(&buffer, &buffer_size, reader->file);
    return buffer;
}

int huffman_bit_reader_read_bit(HuffmanBitReader *reader){
    if (reader->mask == 0) {
        reader->byte = huffman_bit_reader_read_byte(reader);
        reader->mask = 1 << 7;
    }
    int bit = reader->byte & reader->mask;
    reader->mask >>= 1;
    return bit != 0;
}

uint8_t huffman_bit_reader_read_byte(HuffmanBitReader *reader){
    uint8_t byte;
    fread(&byte, sizeof(uint8_t), 1, reader->file);
    return byte;
}