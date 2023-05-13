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
    FILE *img;
    char *flag, *header, *code, *comp;
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
        c = mmap(NULL, pcount, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
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
            code = encode(freq, grey, pcount);
            comp = malloc(stlen + 2);
            for(i = 0; i < stlen - 4; i++){
                comp[i] = argv[2][i];
            }
            comp[stlen - 4] = '\0';
            strcat(comp, ".cbmp");
            huff_writer(fileHeader, infoHeader, header, code, comp);
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
        huff_reader(argv[2], flag);
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

char *encode(int *freqlist, BYTE *c, int pcount){
    huff *hufftree;
    char *codes;
    int i, size;
    char data[256];
    char arr[MAX_TREE_HT];
    size = 0;
    for(i = 0; i < 256; i++){
        data[i] = (char)i;
        if(freqlist[i] != 0){
            size++;
        }
    }
    hufftree = buildHuffmanTree(data, freqlist, size);
    codes = getCodes(hufftree, arr, 0);
    free_huff_tree(hufftree);
    return codes;
}

/*writer and reader stuff*/
void huff_writer(FILEHEADER *fileheader, INFOHEADER *infoheader,char *header, char *codes, char *comp){
    FILE *out = fopen(comp, "wb");
    writeFHeader(fileheader, out);
    writeIHeader(infoheader, out);
    strcat(header, "\n");
    strcat(codes, "\n");
    fputs(header, out);
    fputs(codes, out);
    fclose(out);
}

void huff_reader(char *comp, char *flag){
    FILE *in, *out;
    FILEHEADER *fileheader;
    INFOHEADER *infoheader;
    int width, height, pcount, padding;
    char *header = NULL;
    char *codes = NULL;
    BYTE null;
    BYTE *N;
    size_t header_bufsize = 0;
    size_t codes_bufsize = 0;
    ssize_t header_length, codes_length, j;
    huff *hufftree, *temp;
    BYTE *outdata;
    char data[256], arr[MAX_TREE_HT];
    int i, size, outlen, outind, *freqlist;
    in = fopen(comp, "rb");
    fileheader = malloc(sizeof(FILEHEADER));
    infoheader = malloc(sizeof(INFOHEADER));
    readFHeader(fileheader, in);
    readIHeader(infoheader, in);
    width = infoheader->biWidth;
    height = infoheader->biHeight;
    pcount = width * height;
    padding = (4 - (width * 3) % 4) % 4;
    fseek(in, fileheader->bfOffBits, SEEK_SET);
    header_length = getline(&header, &header_bufsize, in);
    codes_length = getline(&codes, &codes_bufsize, in);
    freqlist = parse_header(header);
    size = 0;
    for(i = 0; i < 256; i++){
        data[i] = (char)i;
        if(freqlist[i] != 0){
            size++;
        }
    }
    hufftree = buildHuffmanTree(data, freqlist, size);
    temp = hufftree;
    outlen = pcount * 3 + height * padding;
    outdata = malloc(outlen);
    outind = 0;
    for(j = 0; j < codes_length; j++){
        if((int)codes[j] == 0){
            temp = temp->left;
        } else if((int)codes[j] == 1){
            temp = temp->right;
        } else{
            continue;
        }
        if(is_leaf(temp)){
            outdata[outind++] = temp->data;
            temp = hufftree;
        }
    }
    out = fopen("out.bmp", "wb");
    writeFHeader(fileheader, out);
    writeIHeader(infoheader, out);
    null = '\0';
    N = &null;
    if(strcmp(flag, "-g") == 0){
        color c, *colors;
        int i;
        BYTE avg;
        colors = malloc(pcount);
        for(i = 0; i < pcount; i++){
            avg = (BYTE)outdata[i];
            c.blue = avg;
            c.green = avg;
            c.red = avg;
            colors[i] = c;
        }
        fwrite(colors, pcount, 1, out);
        // for(i = 0; i < pcount; i++){
        //     fwrite(&colors[i], sizeof(BYTE), 1, out);
        //     if(padding != 0 && (i + 1) % width == 0){
        //         fwrite(N, sizeof(BYTE), padding, out);
        //     }
        // }
        free(colors);
    } else{
        for(i = 0; i < pcount; i++){
            fwrite(&outdata[i], sizeof(color), 1, out);
            if(padding != 0 && (i + 1) % width == 0){
                fwrite(N, sizeof(BYTE), padding, out);
            }
        }
    }
    free(header);
    printf("after header\n");
    free(codes);
    free(fileheader);
    free(infoheader);
    printf("after bmp headers\n");
    //free(outdata);
    printf("after outdata\n");
    free_huff_tree(hufftree);
    printf("after hufftree\n");
    free(freqlist);
    fclose(in);
    fclose(out);
}

void decompress(char *comp, char *flag){
    HuffmanBitReader *read = huffman_bit_reader_init(comp);
    FILE *in, *out;
    FILEHEADER *fileheader;
    INFOHEADER *infoheader;
    int width, height, pcount, padding;
    char *header = NULL;
    char *codes = NULL;
    BYTE null;
    BYTE *N;
    size_t header_bufsize = 0;
    size_t codes_bufsize = 0;
    ssize_t header_length, codes_length, j;
    huff *hufftree, *temp;
    BYTE *result;
    int data[256], arr[MAX_TREE_HT], *token, *copy;
    int i, size, outlen, outind, check, *freqlist;
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
        data[i] = (char)i;
        if(freqlist[i] != 0){
            size++;
        }
    }
    hufftree = buildHuffmanTree(data, freqlist, size);
    temp = hufftree;
    i = 0;
    result = (char *)malloc(size + 1);
    result[size] = '\0';
    while (i < size) {
        check = huffman_bit_reader_read_bit(read);
        if(check == 0){
            temp = temp->left;
        } else if(check == 1){
            temp = temp->right;
        } else{
            continue;
        }
        if (is_leaf(temp)) {
            result[i++] = (char)temp->data;
            temp = hufftree;
        }
    }
    out = fopen("out.bmp", "wb");
    writeFHeader(fileheader, out);
    writeIHeader(infoheader, out);
    null = '\0';
    N = &null;
    if(strcmp(flag, "-g") == 0){
        color c, *colors;
        int i;
        BYTE avg;
        colors = malloc(pcount*sizeof(color));
        for(i = 0; i < pcount; i++){
            avg = (BYTE)result[i];
            printf("%d\n", (int)avg);
            c.blue = avg;
            c.green = avg;
            c.red = avg;
            colors[i] = c;
        }
        fwrite(colors, pcount, 1, out);
        // for(i = 0; i < pcount; i++){
        //     fwrite(&colors[i], sizeof(BYTE), 1, out);
        //     if(padding != 0 && (i + 1) % width == 0){
        //         fwrite(N, sizeof(BYTE), padding, out);
        //     }
        // }
        free(colors);
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
}

struct MinHeap* createMinHeap(unsigned capacity)
 
{
 
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
 
    // current size is 0
    minHeap->size = 0;
 
    minHeap->capacity = capacity;
 
    minHeap->array = (huff**)malloc(minHeap->capacity * sizeof(huff*));
    return minHeap;
}

struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size){
    struct MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

huff* buildHuffmanTree(char data[], int freq[], int size){
    huff *left, *right, *top;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);
    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode('$', left->freq + right->freq);
 
        top->left = left;
        top->right = right;
 
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void printArr(int arr[], int n)
{
    int i;
    for (i = 0; i < n; ++i)
        printf("%d", arr[i]);
 
    printf("\n");
}

char *getCodes(struct MinHeapNode* root, char arr[], int top) {
    char *result = (char *)calloc(1, sizeof(char));
    result[0] = '\0';
    if (root->left) {
        arr[top] = '0';
        char *left_result = getCodes(root->left, arr, top + 1);
        result = (char *)realloc(result, strlen(result) + strlen(left_result) + 1);
        strcat(result, left_result);
        free(left_result);
    }
    if (root->right) {
        arr[top] = '1';
        char *right_result = getCodes(root->right, arr, top + 1);
        result = (char *)realloc(result, strlen(result) + strlen(right_result) + 1);
        strcat(result, right_result);
        free(right_result);
    }
    if (is_leaf(root)) {
        char buffer[256];
        sprintf(buffer, "%s", arr);
        result = (char *)realloc(result, strlen(result) + strlen(buffer) + 1);
        strcat(result, buffer);
    }
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

void huff_bit_writer(char *comp, char *code, char *header){
    HuffmanBitWriter *write = huffman_bit_writer_init(comp);
    huffman_bit_writer_write_str(write, header);
    huffman_bit_writer_write_str(write, "\n");
    huffman_bit_writer_write_code(write, code);
    huffman_bit_writer_close(write);
}