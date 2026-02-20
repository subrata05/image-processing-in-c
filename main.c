#include <stdio.h>
#include "bmp.h"

int main(void) {
    BMPImage_t img;

    /* Load image */
    if (bmpLoad("mr.bean.bmp", &img) != 0) {
        return -1;
    }

    printf("Loaded: %dx%d, %d bits/pixel\n", img.width, img.height, img.bitDepth);

    /* Save reconstructed copy */
    if (bmpSave("img_reconstructed.bmp", &img) != 0) {
        bmpFree(&img);
        return -1;
    }

    printf("Saved to 'img_reconstructed.bmp'\n");

    /* Cleanup */
    bmpFree(&img);
    return 0;
}