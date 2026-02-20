#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BMP_FILE_HEADER_SIZE    14
#define BMP_MIN_INFO_SIZE       40


typedef struct {
    unsigned char fileHeader[BMP_FILE_HEADER_SIZE];
    unsigned char *infoHeader;
    uint32_t infoHeaderSize;
    
    unsigned char *colorTab;
    int colorTabSize;
    
    int32_t width, height;
    uint16_t bitDepth;
    uint32_t offset;
    unsigned char *pixel;
} BMPImage_t;

/* Function prototypes */
int  bmpLoad(const char *filename, BMPImage_t *img);
int  bmpSave(const char *filename, const BMPImage_t *img);
void bmpFree(BMPImage_t *img);
void printUsage(const char *programName);

#endif