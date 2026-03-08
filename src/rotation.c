#include "../include/rotation.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Compiler attribute to mark intentionally unused parameters */
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

/* Helper: Update BMP headers for new dimensions */
static int updateHeadersForRotation(const BMPImage_t *src, BMPImage_t *dst,
                                    int32_t newWidth, int32_t newHeight,
                                    int dstRowSize UNUSED, int dstPixelSize)
{
    /* Copy and update file header */
    memcpy(dst->fileHeader, src->fileHeader, BMP_FILE_HEADER_SIZE);

    /* Update file size: headers + color table + pixel data */
    uint32_t newFileSize = BMP_FILE_HEADER_SIZE + src->infoHeaderSize +
                           src->colorTabSize + dstPixelSize;
    dst->fileHeader[2] = (unsigned char)(newFileSize);
    dst->fileHeader[3] = (unsigned char)(newFileSize >> 8);
    dst->fileHeader[4] = (unsigned char)(newFileSize >> 16);
    dst->fileHeader[5] = (unsigned char)(newFileSize >> 24);

    /* Allocate and copy info header */
    dst->infoHeaderSize = src->infoHeaderSize;
    dst->infoHeader = (unsigned char *)malloc(dst->infoHeaderSize);
    if (!dst->infoHeader)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for info header\n");
        return -1;
    }
    memcpy(dst->infoHeader, src->infoHeader, dst->infoHeaderSize);

    /* Update width and height in info header (little-endian) */
    dst->infoHeader[4] = (unsigned char)(newWidth);
    dst->infoHeader[5] = (unsigned char)(newWidth >> 8);
    dst->infoHeader[6] = (unsigned char)(newWidth >> 16);
    dst->infoHeader[7] = (unsigned char)(newWidth >> 24);

    dst->infoHeader[8] = (unsigned char)(newHeight);
    dst->infoHeader[9] = (unsigned char)(newHeight >> 8);
    dst->infoHeader[10] = (unsigned char)(newHeight >> 16);
    dst->infoHeader[11] = (unsigned char)(newHeight >> 24);

    /* Update image size field (offset 20 for BITMAPINFOHEADER) */
    if (dst->infoHeaderSize >= 24)
    {
        dst->infoHeader[20] = (unsigned char)(dstPixelSize);
        dst->infoHeader[21] = (unsigned char)(dstPixelSize >> 8);
        dst->infoHeader[22] = (unsigned char)(dstPixelSize >> 16);
        dst->infoHeader[23] = (unsigned char)(dstPixelSize >> 24);
    }

    /* Copy color table if present */
    dst->colorTabSize = src->colorTabSize;
    if (dst->colorTabSize > 0)
    {
        dst->colorTab = (unsigned char *)malloc(dst->colorTabSize);
        if (!dst->colorTab)
        {
            fprintf(stderr, "ERROR: Memory allocation failed for color table\n");
            free(dst->infoHeader);
            dst->infoHeader = NULL;
            return -1;
        }
        memcpy(dst->colorTab, src->colorTab, dst->colorTabSize);
    }

    dst->offset = src->offset;
    dst->bitDepth = src->bitDepth;
    dst->width = newWidth;
    dst->height = newHeight;

    return 0;
}

/* Generic pixel copy with coordinate transformation */
static int allocateAndTransformPixels(const BMPImage_t *src, BMPImage_t *dst, int32_t newWidth, int32_t newHeight,
                                      int (*transform)(int srcX, int srcY,
                                                       int srcW, int srcH,
                                                       int *dstX, int *dstY))
{
    int srcWidth = src->width;
    int srcHeight = abs(src->height);
    int dstHeight = abs(newHeight);
    int bytesPerPixel = src->bitDepth / 8;
    int srcRowSize = ((srcWidth * src->bitDepth + 31) / 32) * 4;
    int dstRowSize = ((newWidth * src->bitDepth + 31) / 32) * 4;
    int dstPixelSize = dstRowSize * dstHeight;

    /* Update headers */
    if (updateHeadersForRotation(src, dst, newWidth, newHeight, dstRowSize, dstPixelSize) != 0)
    {
        return -1;
    }

    /* Allocate pixel buffer */
    dst->pixel = (unsigned char *)malloc(dstPixelSize);
    if (!dst->pixel)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for pixel data\n");
        free(dst->colorTab);
        free(dst->infoHeader);
        dst->colorTab = NULL;
        dst->infoHeader = NULL;
        return -1;
    }

    /* Perform transformation */
    for (int y = 0; y < srcHeight; y++)
    {
        for (int x = 0; x < srcWidth; x++)
        {
            int dstX, dstY;
            if (transform(x, y, srcWidth, srcHeight, &dstX, &dstY))
            {
                unsigned char *srcPtr = src->pixel + y * srcRowSize + x * bytesPerPixel;
                unsigned char *dstPtr = dst->pixel + dstY * dstRowSize + dstX * bytesPerPixel;
                memcpy(dstPtr, srcPtr, bytesPerPixel);
            }
        }
    }

    return 0;
}

/* ============ INDIVIDUAL TRANSFORMATION FUNCTIONS ============ */

/* 90 degrees clockwise: (x,y) -> (height-1-y, x) */
static int transform90CW(int srcX, int srcY, int srcW UNUSED, int srcH, int *dstX, int *dstY)
{
    *dstX = srcH - 1 - srcY;
    *dstY = srcX;
    return 1;
}
int rotate90CW(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, abs(src->height), src->width, transform90CW);
}

/* 90 degrees counter-clockwise: (x,y) -> (y, width-1-x) */
static int transform90CCW(int srcX, int srcY, int srcW, int srcH UNUSED, int *dstX, int *dstY)
{
    *dstX = srcY;
    *dstY = srcW - 1 - srcX;
    return 1;
}

int rotate90CCW(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, abs(src->height), src->width, transform90CCW);
}

/* 180 degrees: (x,y) -> (width-1-x, height-1-y) */
static int transform180(int srcX, int srcY, int srcW, int srcH, int *dstX, int *dstY)
{
    *dstX = srcW - 1 - srcX;
    *dstY = srcH - 1 - srcY;
    return 1;
}

int rotate180(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, src->width, src->height, transform180);
}

/* Horizontal flip (mirror): (x,y) -> (width-1-x, y) */
static int transformFlipH(int srcX, int srcY, int srcW, int srcH UNUSED, int *dstX, int *dstY)
{
    *dstX = srcW - 1 - srcX;
    *dstY = srcY;
    return 1;
}

int flipHorizontal(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, src->width, src->height, transformFlipH);
}

/* Vertical flip: (x,y) -> (x, height-1-y) */
static int transformFlipV(int srcX, int srcY, int srcW UNUSED, int srcH, int *dstX, int *dstY)
{
    *dstX = srcX;
    *dstY = srcH - 1 - srcY;
    return 1;
}
int flipVertical(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, src->width, src->height, transformFlipV);
}

/* Transpose (flip along main diagonal): (x,y) -> (y, x) */
static int transformTranspose(int srcX, int srcY, int srcW UNUSED, int srcH UNUSED, int *dstX, int *dstY)
{
    *dstX = srcY;
    *dstY = srcX;
    return 1;
}

int transpose(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, abs(src->height), src->width, transformTranspose);
}

/* Transpose anti-diagonal: (x,y) -> (height-1-y, width-1-x) */
static int transformAntiDiag(int srcX, int srcY, int srcW, int srcH, int *dstX, int *dstY)
{
    *dstX = srcH - 1 - srcY;
    *dstY = srcW - 1 - srcX;
    return 1;
}

int transposeAntiDiagonal(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));
    return allocateAndTransformPixels(src, dst, abs(src->height), src->width, transformAntiDiag);
}

/* ============ UNIFIED INTERFACE ============ */

int rotateImage(const BMPImage_t *src, BMPImage_t *dst, RotationType_t type)
{
    switch (type)
    {
    case ROTATE_90_CW:
    case ROTATE_270_CCW:
        return rotate90CW(src, dst);
    case ROTATE_90_CCW:
    case ROTATE_270_CW:
        return rotate90CCW(src, dst);
    case ROTATE_180:
        return rotate180(src, dst);
    case ROTATE_FLIP_H:
        return flipHorizontal(src, dst);
    case ROTATE_FLIP_V:
        return flipVertical(src, dst);
    case ROTATE_FLIP_DIAG:
        return transpose(src, dst);
    case ROTATE_FLIP_ANTIDIAG:
        return transposeAntiDiagonal(src, dst);
    default:
        fprintf(stderr, "ERROR: Unknown rotation type\n");
        return -1;
    }
}

/* ============ UTILITY FUNCTIONS ============ */

const char *rotationTypeToString(RotationType_t type)
{
    switch (type)
    {
    case ROTATE_90_CW:
        return "90 degrees clockwise";
    case ROTATE_90_CCW:
        return "90 degrees counter-clockwise";
    case ROTATE_180:
        return "180 degrees";
    case ROTATE_270_CW:
        return "270 degrees clockwise";
    case ROTATE_270_CCW:
        return "270 degrees counter-clockwise";
    case ROTATE_FLIP_H:
        return "horizontal flip (mirror)";
    case ROTATE_FLIP_V:
        return "vertical flip";
    case ROTATE_FLIP_DIAG:
        return "transpose (main diagonal)";
    case ROTATE_FLIP_ANTIDIAG:
        return "transpose (anti-diagonal)";
    default:
        return "unknown";
    }
}

int getRotationAngle(RotationType_t type, int *angle, int *clockwise)
{
    switch (type)
    {
    case ROTATE_90_CW:
        *angle = 90;
        *clockwise = 1;
        return 0;
    case ROTATE_90_CCW:
        *angle = 90;
        *clockwise = 0;
        return 0;
    case ROTATE_180:
        *angle = 180;
        *clockwise = 1;
        return 0;
    case ROTATE_270_CW:
        *angle = 270;
        *clockwise = 1;
        return 0;
    case ROTATE_270_CCW:
        *angle = 270;
        *clockwise = 0;
        return 0;
    case ROTATE_FLIP_H:
    case ROTATE_FLIP_V:
    case ROTATE_FLIP_DIAG:
    case ROTATE_FLIP_ANTIDIAG:
        *angle = 0;
        *clockwise = 0;
        return 0;
    default:
        return -1;
    }
}

RotationType_t parseRotationString(const char *str)
{
    if (strcmp(str, "90cw") == 0 || strcmp(str, "90") == 0 ||
        strcmp(str, "right") == 0 || strcmp(str, "r") == 0)
        return ROTATE_90_CW;
    if (strcmp(str, "90ccw") == 0 || strcmp(str, "left") == 0 ||
        strcmp(str, "l") == 0)
        return ROTATE_90_CCW;
    if (strcmp(str, "180") == 0 || strcmp(str, "flip") == 0)
        return ROTATE_180;
    if (strcmp(str, "270cw") == 0 || strcmp(str, "270") == 0)
        return ROTATE_270_CW;
    if (strcmp(str, "270ccw") == 0)
        return ROTATE_270_CCW;
    if (strcmp(str, "hflip") == 0 || strcmp(str, "mirror") == 0 ||
        strcmp(str, "h") == 0)
        return ROTATE_FLIP_H;
    if (strcmp(str, "vflip") == 0 || strcmp(str, "v") == 0)
        return ROTATE_FLIP_V;
    if (strcmp(str, "transpose") == 0 || strcmp(str, "diag") == 0 ||
        strcmp(str, "t") == 0)
        return ROTATE_FLIP_DIAG;
    if (strcmp(str, "antidiag") == 0 || strcmp(str, "at") == 0)
        return ROTATE_FLIP_ANTIDIAG;

    return (RotationType_t)-1; /* Invalid */
}

/* ============ BATCH OPERATIONS ============ */

int rotateBatch(const BMPImage_t *src, BMPImage_t *dst,
                const RotationType_t *operations, int numOps)
{
    if (numOps <= 0)
    {
        fprintf(stderr, "ERROR: No operations specified for batch rotation\n");
        return -1;
    }

    BMPImage_t temp1, temp2;
    const BMPImage_t *currentSrc = src;
    BMPImage_t *currentDst = &temp1;
    int useTemp1 = 1;

    for (int i = 0; i < numOps; i++)
    {
        memset(currentDst, 0, sizeof(BMPImage_t));

        if (rotateImage(currentSrc, currentDst, operations[i]) != 0)
        {
            /* Cleanup on failure */
            if (currentSrc != src)
                bmpFree((BMPImage_t *)currentSrc);
            return -1;
        }

        printf("  Step %d: %s\n", i + 1, rotationTypeToString(operations[i]));

        /* Free previous temp if not the original source */
        if (currentSrc != src)
        {
            bmpFree((BMPImage_t *)currentSrc);
        }

        /* Swap buffers for next iteration */
        currentSrc = currentDst;
        currentDst = useTemp1 ? &temp2 : &temp1;
        useTemp1 = !useTemp1;
    }

    /* Copy final result to dst */
    memcpy(dst, currentSrc, sizeof(BMPImage_t));

    /* Clear the temp struct without freeing (ownership transferred to dst) */
    if (currentSrc == &temp1)
    {
        memset(&temp1, 0, sizeof(BMPImage_t));
    }
    else
    {
        memset(&temp2, 0, sizeof(BMPImage_t));
    }

    return 0;
}