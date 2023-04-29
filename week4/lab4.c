#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "lab4.h"

int main(int argc, char *argv[]){
    FILE *img, *out;
    float bright;
    int parallel;
    BYTE *pixel;
    int width, height;
    int pcount, padding;
    color *c;
    int x, y, i, n, h, pix;
    BYTE null;
    BYTE *N;
    FILEHEADER *fileHeader;
    INFOHEADER *infoHeader;
    clock_t start, end;
    double time;

    img = fopen(argv[1], "rb");
    out = fopen(argv[4], "wb");
    bright = atof(argv[2]);
    parallel = atoi(argv[3]);
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
    for(y = 0; y < height; y++){
        for(x = 0; x < width; x++){
            c[y * width + x] = get_color(pixel, width, height, x, y, padding);
        }
    }
    if(parallel == 0){
        for(i = 0; i < pcount; i++){
            c[i] = brighten(c[i], bright);
        }
        writeFHeader(fileHeader, out);
        writeIHeader(infoHeader, out);
        null = '\0';
        N = &null;
        for(i = 0; i < pcount; i++){
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
    } else{
        start = clock();
        n = fork();
        h = (int)(height / 2);
        if(height % 2 != 0){
            h += 1;
        }
        pix = width * h;
        if(n == 0){
            for(i = 0; i < pix; i++){
                c[i] = brighten(c[i], bright);
            }
        } else{
            wait(0);
            for(i = pix; i < pcount; i++){
                c[i] = brighten(c[i], bright);
            }
            writeFHeader(fileHeader, out);
            writeIHeader(infoHeader, out);
            null = '\0';
            N = &null;
            for(i = 0; i < pcount; i++){
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
            end = clock();
            time = (double)(end - start) / CLOCKS_PER_SEC;
            printf("Time taken: %f\n", time);
        }
    }
    return 0;
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

color brighten(color c, float ratio){
    int b, g, r;
    b = (int)c.blue + (ratio * 255);
    g = (int)c.green + (ratio * 255);
    r = (int)c.red + (ratio * 255);
    if(b > 255){
        b = 255;
    }
    if(g > 255){
        g = 255;
    }
    if(r > 255){
        r = 255;
    }
    c.blue = (BYTE)b;
    c.green = (BYTE)g;
    c.red = (BYTE)r;
    return c;
}
