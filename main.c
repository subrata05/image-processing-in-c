#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE    54

typedef struct
{
    unsigned char header[SIZE];

    unsigned char *colorTab;
    int colorTabSize;
    
    int32_t width, height;
    uint16_t bitDepth;
    uint32_t offset, imgSize;
    unsigned char *pixel;
} BMPImg_t;


int main(void) {
    BMPImg_t img;

    /*----- file reading process -----*/

    FILE *sourceImg = fopen("mr.bean.bmp", "rb");

    if(sourceImg == NULL) {
        printf("ERROR: Missing source file!\n");
        return -1;
    }

    if(fread(img.header, 1, SIZE, sourceImg) != SIZE) {
        printf("ERROR: Corrupted file!\n");
        fclose(sourceImg);
        return -1;
    }

    /*----- feature exctracting process -----*/

    img.offset      = *(uint32_t *)&img.header[10];
    img.width       = *(int32_t  *)&img.header[18];
    img.height      = *(int32_t  *)&img.header[22];
    img.bitDepth    = *(uint16_t *)&img.header[28];

    img.colorTabSize = img.offset - SIZE;

    if(img.colorTabSize > 0) {
        img.colorTab = (unsigned char*)malloc(img.colorTabSize);
        fread(img.colorTab, 1, img.colorTabSize, sourceImg);
    } else {
        img.colorTab = NULL;
    }

    int rowSize = ((img.width * img.bitDepth + 31) / 32) * 4;   // calculate Pixel Data Size (Row Padding)
    int pixelArrSize = rowSize * abs(img.height);

    img.pixel = (unsigned char*)malloc(pixelArrSize);

    fseek(sourceImg, img.offset, SEEK_SET);
    fread(img.pixel, 1, pixelArrSize, sourceImg);
    fclose(sourceImg);

    printf("Source image loaded succesfully!\n");
    printf("Width: %d, Height: %d, Depth: %d\n", img.width, img.height, img.bitDepth);

    /*----- reconstruction process -----*/

    FILE *desinationImg = fopen("img_reconstructed.bmp", "wb");

    fwrite(img.header, 1, 54, desinationImg);

    if (img.colorTabSize > 0) {
        fwrite(img.colorTab, 1, img.colorTabSize, desinationImg);
    }

    fwrite(img.pixel, 1, pixelArrSize, desinationImg);
    fclose(desinationImg);

    printf("Image successfully reconstructed and saved in \"img_reconstructed.bmp\" file.\n");

    /*----- clean up -----*/

    if (img.colorTab) free(img.colorTab);   
    free(img.pixel);

    return 0;
}