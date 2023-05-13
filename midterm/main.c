#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "main.h"

int main(int argc, char *argv[]){
    FILE *img;
    char *flag, *header, *code, *comp;
    BYTE *pixel, *grey;
    int width, height;
    int pcount, padding;
    color *c;
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
            code = encode(freq, header);
            comp = malloc(stlen + 2);
            for(i = 0; i < stlen - 4; i++){
                comp[i] = argv[2][i];
            }
            comp[stlen - 4] = '\0';
            strcat(comp, ".cbmp");
            huff_bit_writer(fileHeader, infoHeader, comp, code, header);
            free(freq);
            free(comp);
            free(header);
            free(code);
            fclose(img);
            munmap(fileHeader, sizeof(FILEHEADER));
            munmap(infoHeader, sizeof(INFOHEADER));
            munmap(pixel, pcount * 3 + height * padding);
            munmap(c, pcount * sizeof(color));
            munmap(grey, pcount * sizeof(BYTE));
        }
    } else{
        decompress(argv[2], flag);
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

char *encode(int *freqlist, char *header){
    huff *hufftree;
    char *codes, *copy, *token;
    int i, size, char_count, count, scount, *freq, *data;
    char arr[MAX_TREE_HT];
    size = 0;
    for(i = 0; i < 256; i++){
        if(freqlist[i] != 0){
            size++;
        }
    }
    data = malloc(size * sizeof(int));
    freq = malloc(size * sizeof(int));
    char_count = 0;
    count = 0;
    copy = strdup(header);
    token = strtok(copy, " ");
    for(i = 1; token != NULL; i++){
        if(i % 2 == 0){
            char_count += atoi(token);
            freq[count] = atoi(token);
            count++;
        }
        token = strtok(NULL, " ");
    }
    count = 0;
    scount = 0;
    copy = strdup(header);
    token = strtok(copy, " ");
    while (token != NULL) {
        if (count % 2 == 0) {
            data[scount] = atoi(token);
            scount++;
        }
        count++;
        token = strtok(NULL, " ");
    }

    hufftree = buildHuffmanTree(data, freq, size);
    codes = getCodes(hufftree);
    free(data);
    free(freq);
    free(copy);
    free_huff_tree(hufftree);
    return codes;
}

void decompress(char *comp, char *flag){
    HuffmanBitReader *read = huffman_bit_reader_init(comp);
    FILE *in, *out;
    FILEHEADER *fileheader;
    INFOHEADER *infoheader;
    int width, height, pcount, padding;
    char *header;
    BYTE null;
    BYTE *N;
    size_t header_bufsize = 0;
    size_t codes_bufsize = 0;
    ssize_t header_length, codes_length, j;
    huff *hufftree, *temp;
    BYTE *result;
    char arr[MAX_TREE_HT], *token, *copy, *code;
    int i, size, count, scount, char_count, chars, check, *freqlist, *freq, *data;
    in = read->file;
    fileheader = malloc(sizeof(FILEHEADER));
    infoheader = malloc(sizeof(INFOHEADER));
    readFHeader(fileheader, in);
    readIHeader(infoheader, in);
    width = infoheader->biWidth;
    height = infoheader->biHeight;
    pcount = width * height;
    padding = (4 - (width * 3) % 4) % 4;
    fseek(in, fileheader->bfOffBits, SEEK_SET);
    header = huffman_bit_reader_read_str(read);
    freqlist = parse_header(header);
    size = 0;
    for(i = 0; i < 256; i++){
        if(freqlist[i] != 0){
            size++;
        }
    }
    data = malloc(size * sizeof(int));
    freq = malloc(size * sizeof(int));
    char_count = 0;
    count = 0;
    copy = strdup(header);
    free(copy);
    token = strtok(copy, " ");
    for(i = 1; token != NULL; i++){
        if(i % 2 == 0){
            char_count += atoi(token);
            freq[count] = atoi(token);
            count++;
        }
        token = strtok(NULL, " ");
    }
    count = 0;
    scount = 0;
    copy = strdup(header);
    token = strtok(copy, " ");
    while (token != NULL) {
        if (count % 2 == 0) {
            data[scount] = atoi(token);
            scount++;
        }
        count++;
        token = strtok(NULL, " ");
    }
    hufftree = buildHuffmanTree(data, freq, size);
    temp = hufftree;
    chars = 0;
    result = (char *)malloc(char_count + 1);
    i = 0;
    code = read_bits_as_string(read->file);
    while(chars < char_count){
        if(code[i] == '0'){
            temp = temp->left;
        } else{
            temp = temp->right;
        }
        if (temp->left == NULL && temp->right == NULL){
            result[chars++] = (char)temp->data;
            temp = hufftree;
        }
        i++;
    }
    out = fopen("out.bmp", "wb");
    writeFHeader(fileheader, out);
    writeIHeader(infoheader, out);
    null = '\0';
    N = &null;
    if(strcmp(flag, "-g") == 0){
        int i;
        int avg;
        for(i = 0; i < pcount; i++){
            color *c = malloc(sizeof(color));
            avg = (int)result[i];
            c->blue = (BYTE)avg;
            c->green = (BYTE)avg;
            c->red = (BYTE)avg;
            fwrite(c, sizeof(color), 1, out);
            if(padding != 0 && (i + 1) % width == 0){
                fwrite(N, sizeof(BYTE), padding, out);
            }
            free(c);
        }
    } else{
        for(i = 0; i < pcount; i++){
            fwrite(&result[i], sizeof(color), 1, out);
            if(padding != 0 && (i + 1) % width == 0){
                fwrite(N, sizeof(BYTE), padding, out);
            }
        }
    }
    free(result);
    huffman_bit_reader_close(read);
    free(header);
    free(freqlist);
    free(fileheader);
    free(infoheader);
    free_huff_tree(hufftree);
    free(data);
    free(freq);
    free(copy);
    free(code);
    fclose(out);
}

/*min heap stuff*/
void swapMinHeapNode(huff** a, huff** b){
    huff* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(struct MinHeap* minHeap, int idx){
 
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
 
    if (left < minHeap->size
        && minHeap->array[left]->freq
               < minHeap->array[smallest]->freq)
        smallest = left;
 
    if (right < minHeap->size
        && minHeap->array[right]->freq
               < minHeap->array[smallest]->freq)
        smallest = right;
 
    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest],
                        &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(struct MinHeap* minHeap){
    return (minHeap->size == 1);
}

huff* extractMin(struct MinHeap* minHeap){
    huff* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(struct MinHeap* minHeap, huff* minHeapNode){
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i
           && minHeapNode->freq
                  < minHeap->array[(i - 1) / 2]->freq) {
 
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

void buildMinHeap(struct MinHeap* minHeap){
    int n = minHeap->size - 1;
    int i;
    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int is_leaf(huff* root){
    if(root->left == NULL && root->right == NULL){
        return 1;
    }
    return 0;
}

struct MinHeap* createMinHeap(unsigned capacity)
 
{
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->size = 0;
 
    minHeap->capacity = capacity;
 
    minHeap->array = (huff**)malloc(minHeap->capacity * sizeof(huff*));
    return minHeap;
}

struct MinHeap* createAndBuildMinHeap(int data[], int freq[], int size){
    struct MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

huff* buildHuffmanTree(int data[], int freq[], int size){
    huff *left, *right, *top;
    int d;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);
    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        if(left->data < right->data){
            d = left->data;
        } else if(right->data < left->data){
            d = right->data;
        }
        top = newNode(d, left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void getCodesHelper(struct MinHeapNode* root, int arr[], int top, char **result) {

    if (root->left) {
        arr[top] = 0;
        getCodesHelper(root->left, arr, top + 1, result);
    }

    if (root->right) {
        arr[top] = 1;
        getCodesHelper(root->right, arr, top + 1, result);
    }

    if (is_leaf(root)) {
        for (int i = 0; i < top; ++i) {
            char bit = arr[i] + '0';
            size_t len = strlen(*result);
            *result = (char *)realloc(*result, len + 2);
            (*result)[len] = bit;
            (*result)[len + 1] = '\0';
        }
    }
}

char *getCodes(struct MinHeapNode *root) {
    int arr[MAX_TREE_HT], top = 0;
    char *result = (char *)calloc(1, sizeof(char));
    result[0] = '\0';
    getCodesHelper(root, arr, top, &result);
    return result;
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

char *read_bits_as_string(FILE *file){
    size_t buffer_size = 4096;
    size_t buffer_pos = 0;
    char *bit_str = (char *)malloc(buffer_size);
    uint8_t current_byte;
    while (fread(&current_byte, sizeof(uint8_t), 1, file) == 1) {
        for (int bit_position = 0; bit_position < 8; bit_position++) {
            if (buffer_pos + 1 >= buffer_size) {
                buffer_size *= 2;
                bit_str = (char *)realloc(bit_str, buffer_size);
            }

            int current_bit = (current_byte >> (7 - bit_position)) & 1;
            bit_str[buffer_pos++] = current_bit ? '1' : '0';
        }
    }
    if (buffer_pos + 1 >= buffer_size) {
        buffer_size += 1;
        bit_str = (char *)realloc(bit_str, buffer_size);
    }
    bit_str[buffer_pos] = '\0';
    return bit_str;
}

void huff_bit_writer(FILEHEADER *fileheader, INFOHEADER *infoheader, char *comp, char *code, char *header){
    HuffmanBitWriter *write = huffman_bit_writer_init(comp);
    writeFHeader(fileheader, write->file);
    writeIHeader(infoheader, write->file);
    huffman_bit_writer_write_str(write, header);
    huffman_bit_writer_write_str(write, "\n");
    huffman_bit_writer_write_code(write, code);
    huffman_bit_writer_close(write);
}


huff* newNode(int data, unsigned freq)
{
    huff* temp = (huff*)malloc(
        sizeof(huff));
 
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
 
    return temp;
}

