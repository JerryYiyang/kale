#include <stdio.h>
#include <stdlib.h>

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

void invalidInput();

color get_color(BYTE *pixel, int width, int height, int x, int y, int padding);

void readFHeader(FILEHEADER *fh, FILE *f);
void readIHeader(INFOHEADER *ih, FILE *f);
void writeFHeader(FILEHEADER *fh, FILE *f);
void writeIHeader(INFOHEADER *ih, FILE *f);

int clamp(int value);

color bilinear_interpolation(BYTE *pixel, int width, int height, float x, float y, int padding);