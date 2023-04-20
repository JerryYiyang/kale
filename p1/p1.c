#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "p1.h"

int main(int argc, char *argv[]) {
    FILE *img1, *img2, *out;
    float ratio;
    BYTE *pixel1, *pixel2;
    int width1, height1, width2, height2;
    int pcount1, pcount2;
    int padding1, padding2;
    color *c1, *c2, *c3;
    int x, y, i;
    FILEHEADER *fileHeader1, *fileHeader2;
    INFOHEADER *infoHeader1, *infoHeader2;
    if (argc != 5) {
        printf("Missing parameters, expected:\n");
        printf("[programname] [imagefile1] [imagefile2] [ratio] [outputfile]\n");
        return 1;
    }
    ratio = atof(argv[3]);
    if (ratio < 0.0 || ratio > 1.0) {
        printf("Invalid ratio, should be between 0.0 and 1.0\n");
        return 1;
    }
    img1 = fopen(argv[1], "rb");
    if (!img1) {
        printf("Cannot open file: %s\n", argv[1]);
        return 1;
    }
    img2 = fopen(argv[2], "rb");
    if (!img2) {
        printf("Cannot open file: %s\n", argv[2]);
        fclose(img1);
        return 1;
    }
    out = fopen(argv[4], "wb");
    if (!out) {
        printf("Cannot create output file: %s\n", argv[4]);
        fclose(img1);
        fclose(img2);
        return 1;
    }
    fileHeader1 = malloc(sizeof(FILEHEADER));
    fileHeader2 = malloc(sizeof(FILEHEADER));
    infoHeader1 = malloc(sizeof(INFOHEADER));
    infoHeader2 = malloc(sizeof(INFOHEADER));
    readFHeader(fileHeader1, img1);
    readFHeader(fileHeader2, img2);
    readIHeader(infoHeader1, img1);
    readIHeader(infoHeader2, img2);
    if (fileHeader1->bfType != 0x4D42 || fileHeader2->bfType != 0x4D42) {
        printf("Invalid file format. Only 24-bit BMP files are supported.\n");
        fclose(img1);
        fclose(img2);
        fclose(out);
        return 1;
    }
    /*getting width and height in pixels*/
    width1 = infoHeader1->biWidth;
    height1 = infoHeader1->biHeight;
    width2 = infoHeader2->biWidth;
    height2 = infoHeader2->biHeight;
    /*getting number of pixels*/
    pcount1 = width1 * height1;
    pcount2 = width2 * height2;
    /*getting padding*/
    padding1 = (4 - (width1 * 3) % 4);
    padding2 = (4 - (width2 * 3) % 4);
    /*skipping color table if it exists*/
    fseek(img1, fileHeader1->bfOffBits, SEEK_SET);
    fseek(img2, fileHeader2->bfOffBits, SEEK_SET);
    /*reading image data into array of BYTES*/
    pixel1 = (BYTE *)malloc(pcount1 * 3 + height1 * padding1);
    pixel2 = (BYTE *)malloc(pcount2 * 3 + height2 * padding2);
    fread(pixel1, 1, pcount1 * 3 + height1 * padding1, img1);
    fread(pixel2, 1, pcount2 * 3 + height2 * padding2, img2);
    /*getting pixel colors*/
    c1 = malloc(pcount1 * sizeof(color));
    c2 = malloc(pcount2 * sizeof(color));
    c3 = malloc(pcount1 * sizeof(color));
    for(y = 0; y < height1; y++){
        for(x = 0; x < width1; x++){
            c1[y * width1 + x] = get_color(pixel1, width1, height1, x, y, padding1);
        }
    }
    for(y = 0; y < height2; y++){
        for(x = 0; x < width2; x++){
            c2[y * width2 + x] = get_color(pixel2, width2, height2, x, y, padding2);
        }
    }
    /*mixing colors*/
    for(i = 0; i < pcount1; i++){
        color c;
        c.blue = (c1[i]).blue * ratio + (c2[i]).blue * (1 - ratio);
        c.green = (c1[i]).green * ratio + (c2[i]).green * (1 - ratio);
        c.red = (c1[i]).red * ratio + (c2[i]).red * (1 - ratio);
        c3[i] = c;
    }
    writeFHeader(fileHeader1, out);
    writeIHeader(infoHeader1, out);
    for (y = 0; y < height1; y++) {
        for (x = 0; x < width1; x++) {
            fwrite(&c3[y * width1 + x], sizeof(color), 1, out);
        }
        // Write padding
        BYTE paddingBytes[] = {0, 0, 0};
        fwrite(paddingBytes, 1, padding1, out);
    }
    /*freeing and closing everything*/
    free(c1);
    free(c2);
    free(c3);
    free(fileHeader1);
    free(fileHeader2);
    free(infoHeader1);
    free(infoHeader2);
    free(pixel1);
    free(pixel2);
    fclose(img1);
    fclose(img2);
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
    fread(&(ih->biClrImportant), sizeof(DWORD), 1, f);
    fread(&(ih->biClrUsed), sizeof(DWORD), 1, f);
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
    fwrite(&(ih->biClrImportant), sizeof(DWORD), 1, f);
    fwrite(&(ih->biClrUsed), sizeof(DWORD), 1, f);
}

color get_color(BYTE *pixel, int width, int height, int x, int y, int padding){
    color c;
    int i;
    i = (y * (width * 3 + padding) + x * 3);
    c.blue = pixel[i];
    c.green = pixel[i + 1];
    c.red = pixel[i + 2];
    return c;
}