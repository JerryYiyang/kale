#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAX_TREE_HT 100

/*bitmap files*/
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;
typedef struct color color;
typedef struct tagBITMAPFILEHEADER FILEHEADER;
typedef struct tagBITMAPINFOHEADER INFOHEADER;

struct tagBITMAPFILEHEADER
{
    WORD bfType; /*specifies the file type*/
    DWORD bfSize; /*specifies the size in bytes of the bitmap file*/
    WORD bfReserved1; /*reserved; must be 0*/
    WORD bfReserved2; /*reserved; must be 0*/
    DWORD bfOffBits; /*species the offset in bytes from the bitmapfileheader to*/
};
struct tagBITMAPINFOHEADER
{
    DWORD biSize; /*specifies the number of bytes required by the struct*/
    LONG biWidth; /*specifies width in pixels*/
    LONG biHeight; /*species height in pixels*/
    WORD biPlanes; /*specifies the number of color planes, must be 1*/
    WORD biBitCount; /*specifies the number of bit per pixel*/
    DWORD biCompression;/*spcifies the type of compression*/
    DWORD biSizeImage; /*size of image in bytes*/
    LONG biXPelsPerMeter; /*number of pixels per meter in x axis*/
    LONG biYPelsPerMeter; /*number of pixels per meter in y axis*/
    DWORD biClrUsed; /*number of colors used by th ebitmap*/
    DWORD biClrImportant; /*number of colors that are important*/
};
struct color
{
    BYTE blue, green, red;
};

color get_color(BYTE *pixel, int width, int height, int x, int y, int padding);
BYTE make_grey(color c);

void readFHeader(FILEHEADER *fh, FILE *f);
void readIHeader(INFOHEADER *ih, FILE *f);
void writeFHeader(FILEHEADER *fh, FILE *f);
void writeIHeader(INFOHEADER *ih, FILE *f);

/*huffman stuff*/
typedef struct MinHeapNode huff;
typedef struct HuffmanBitWriter HuffmanBitWriter;
typedef struct HuffmanBitReader HuffmanBitReader;

struct HuffmanBitWriter{
    FILE *file;
    int n_bits;
    uint8_t byte;
};

struct HuffmanBitReader{
    FILE *file;
    int n_bits;
    uint8_t byte;
    uint8_t mask;
};

int *cnt_freq(BYTE *c, int pcount);
void free_code(char **lst);
void free_huff_tree(huff *node);
void help_free_huff_tree(huff *node);
char *create_header(int *freq);
int *parse_header(const char *header_string);
/*char *encode(int *freqlist, char *header);*/
char *encode(int *freqlist, char *header, BYTE *grey, int pcount);
HuffmanBitWriter *huffman_bit_writer_init(const char *fname);
void huffman_bit_writer_close(HuffmanBitWriter *writer);
void huffman_bit_writer_write_str(HuffmanBitWriter *writer, const char *str);
void huffman_bit_writer_write_code(HuffmanBitWriter *writer, const char *code);
HuffmanBitReader *huffman_bit_reader_init(const char *fname);
void huffman_bit_reader_close(HuffmanBitReader *reader);
char *huffman_bit_reader_read_str(HuffmanBitReader *reader);
void huff_writer(FILEHEADER *fileheader, INFOHEADER *infoheader,char *header, char *codes, char *comp);
void huff_bit_writer(FILEHEADER *fileheader, INFOHEADER *infoheader, char *comp, char *code, char *header);
void decompress(char *comp, char *flag);
char *read_bits_as_string(FILE *file);


/*min heap stuff*/
struct MinHeapNode {
    int data;
    unsigned int freq;
    huff *left, *right;
};

struct MinHeap {
    unsigned size;
    unsigned capacity;
    huff** array;
};

void swapMinHeapNode(huff** a, huff** b);
void minHeapify(struct MinHeap* minHeap, int idx);
int isSizeOne(struct MinHeap* minHeap);
huff* extractMin(struct MinHeap* minHeap);
void insertMinHeap(struct MinHeap* minHeap, huff* minHeapNode);
void buildMinHeap(struct MinHeap* minHeap);
int is_leaf(huff* root);
struct MinHeap* createAndBuildMinHeap(int data[], int freq[], int size);
huff* buildHuffmanTree(int data[], int freq[], int size);
char *getCodes(struct MinHeapNode *root);
void getCodesHelper(struct MinHeapNode* root, int arr[], int top, char **result);
huff* newNode(int data, unsigned freq);

char **create_code(huff *node);
void code_help(huff *node, char **lst, char *path);
char *encode_help(int *freqlist, BYTE *grey, int num_pixels, huff *hufftree);

ssize_t getline(char **lineptr, size_t *n, FILE *stream);
char *my_strdup(const char *s);