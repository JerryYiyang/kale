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
    padding1 = (4 - (width1 * 3) % 4) % 4;
    padding2 = (4 - (width2 * 3) % 4) % 4;
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
    if (width1 == width2 && height1 == height2){
        /*mixing colors*/
        c3 = malloc(pcount1 * sizeof(color));
        for(i = 0; i < pcount1; i++){
            color c;
            c.blue = (c1[i]).blue * ratio + (c2[i]).blue * (1 - ratio);
            c.green = (c1[i]).green * ratio + (c2[i]).green * (1 - ratio);
            c.red = (c1[i]).red * ratio + (c2[i]).red * (1 - ratio);
            c3[i] = c;
        }
        writeFHeader(fileHeader1, out);
        writeIHeader(infoHeader1, out);
        fwrite(c3, 1, pcount1 * 3 + height1 * padding1, out);
        free(c3);
    } else{
        int bwidth, bheight, swidth, sheight, bcount, bpadding, spadding;
        FILEHEADER *bfh;
        INFOHEADER *bih;
        BYTE *spixel;
        float rX, rY, sX, sY;
        int x1, y1, x2, y2;
        int dx, dy;
        color *resize;
        if(pcount1 > pcount2){
            c3 = malloc(pcount1 * sizeof(color));
            bwidth = width1;
            bheight = height1;
            swidth = width2;
            sheight = height2;
            bcount = pcount1;
            bpadding = padding1;
            spadding = padding2;
            bfh = fileHeader1;
            bih = infoHeader1;
            spixel = pixel2;
            /*getting ratios*/
            rX = width2 / width1;
            rY = height2 / height1;
            resize = malloc(bcount * sizeof(color));
        } else{
            c3 = malloc(pcount2 * sizeof(color));
            bwidth = width2;
            bheight = height2;
            swidth = width1;
            sheight = height1;
            bcount = pcount2;
            bpadding = padding2;
            spadding = padding1;
            bfh = fileHeader2;
            bih = infoHeader2;
            spixel = pixel1;
            /*getting ratios*/
            rX = (float)width1 / (float)width2;
            rY = (float)height1 / (float)height2;
            resize = malloc(bcount * sizeof(color));
        }
        /*resizing smaller image using the scale*/
        for(y = 0; y < bheight; y++){
            for(x = 0; x < bwidth; x++){
                sX = x * rX;
                sY = y * rY;
                resize[y * bwidth + x] = bilinear_interpolation(spixel, swidth, sheight, sX, sY, spadding);
            }
            /*mixing colors*/
            if(pcount1 > pcount2){
                for(i = 0; i < bcount; i++){
                    color c;
                    c.blue = clamp((c1[i]).blue * ratio + (resize[i]).blue * (1 - ratio));
                    c.green = clamp((c1[i]).green * ratio + (resize[i]).green * (1 - ratio));
                    c.red = clamp((c1[i]).red * ratio + (resize[i]).red * (1 - ratio));
                    c3[i] = c;
                }
            } else{
                for(i = 0; i < bcount; i++){
                    color c;
                    c.blue = clamp((c2[i]).blue * ratio + (resize[i]).blue * (1 - ratio));
                    c.green = clamp((c2[i]).green * ratio + (resize[i]).green * (1 - ratio));
                    c.red = clamp((c2[i]).red * ratio + (resize[i]).red * (1 - ratio));
                    c3[i] = c;
                }
            }
        }
        writeFHeader(bfh, out);
        writeIHeader(bih, out);
        fwrite(c3, 1, bcount * 3 + bheight * bpadding, out);
        free(resize);
        free(c3);
    }
    /*freeing and closing everything that hasn't been done yet*/
    free(c1);
    free(c2);
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

int clamp(int value) {
    if (value < 0) {
        return 0;
    } else if (value > 255) {
        return 255;
    } else {
        return value;
    }
}

color bilinear_interpolation(BYTE *pixel, int width, int height, float x, float y, int padding) {
    int x1 = (int)x;
    int x2 = x1 + 1;
    int y1 = (int)y;
    int y2 = y1 + 1;

    float dx = x - x1;
    float dy = y - y1;

    color cRU = get_color(pixel, width, height, x1, y2, padding);
    color cLU = get_color(pixel, width, height, x2, y2, padding);
    color cLL = get_color(pixel, width, height, x1, y1, padding);
    color cRL = get_color(pixel, width, height, x2, y1, padding);

    color result;
    result.red = (1 - dy) * ((1 - dx) * cLL.red + dx * cRL.red) + dy * ((1 - dx) * cLU.red + dx * cRU.red);
    result.green = (1 - dy) * ((1 - dx) * cLL.green + dx * cRL.green) + dy * ((1 - dx) * cLU.green + dx * cRU.green);
    result.blue = (1 - dy) * ((1 - dx) * cLL.blue + dx * cRL.blue) + dy * ((1 - dx) * cLU.blue + dx * cRU.blue);

    return result;
}
