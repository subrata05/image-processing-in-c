#ifndef ROTATION_H
#define ROTATION_H

#include "bmp.h"

/* Rotation angles */
typedef enum
{
    ROTATE_90_CW,        /* 90 degrees clockwise */
    ROTATE_90_CCW,       /* 90 degrees counter-clockwise */
    ROTATE_180,          /* 180 degrees */
    ROTATE_270_CW,       /* 270 degrees clockwise (same as 90 CCW) */
    ROTATE_270_CCW,      /* 270 degrees counter-clockwise (same as 90 CW) */
    ROTATE_FLIP_H,       /* Horizontal flip (mirror) */
    ROTATE_FLIP_V,       /* Vertical flip */
    ROTATE_FLIP_DIAG,    /* Flip along main diagonal (transpose) */
    ROTATE_FLIP_ANTIDIAG /* Flip along anti-diagonal */
} RotationType_t;

/* Function prototypes */

/* Main rotation function - handles all rotation types */
int rotateImage(const BMPImage_t *src, BMPImage_t *dst, RotationType_t type);

/* Individual rotation functions (for direct use) */
int rotate90CW(const BMPImage_t *src, BMPImage_t *dst);
int rotate90CCW(const BMPImage_t *src, BMPImage_t *dst);
int rotate180(const BMPImage_t *src, BMPImage_t *dst);
int flipHorizontal(const BMPImage_t *src, BMPImage_t *dst);
int flipVertical(const BMPImage_t *src, BMPImage_t *dst);
int transpose(const BMPImage_t *src, BMPImage_t *dst);             /* Flip diagonal */
int transposeAntiDiagonal(const BMPImage_t *src, BMPImage_t *dst); /* Flip anti-diagonal */

/* Utility functions */
const char *rotationTypeToString(RotationType_t type);
int getRotationAngle(RotationType_t type, int *angle, int *clockwise);
RotationType_t parseRotationString(const char *str);

/* Batch rotation - multiple operations in sequence */
int rotateBatch(const BMPImage_t *src, BMPImage_t *dst,
                const RotationType_t *operations, int numOps);

#endif