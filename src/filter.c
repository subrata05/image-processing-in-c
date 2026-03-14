#include "../include/filter.h"
#include <string.h>
#include <stdlib.h>

/* copy src image structure into dst (pixel data included) */
static int cloneImage(const BMPImage_t *src, BMPImage_t *dst)
{
    memset(dst, 0, sizeof(BMPImage_t));

    int srcHeight  = abs(src->height);
    int srcRowSize = ((src->width * src->bitDepth + 31) / 32) * 4;
    int pixelSize  = srcRowSize * srcHeight;

    /* file header */
    memcpy(dst->fileHeader, src->fileHeader, BMP_FILE_HEADER_SIZE);

    /* info header */
    dst->infoHeaderSize = src->infoHeaderSize;
    dst->infoHeader = (unsigned char *)malloc(dst->infoHeaderSize);
    if (!dst->infoHeader)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for info header\n");
        return -1;
    }
    memcpy(dst->infoHeader, src->infoHeader, dst->infoHeaderSize);

    /* color table (present for 8 bpp) */
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

    /* pixel data */
    dst->pixel = (unsigned char *)malloc(pixelSize);
    if (!dst->pixel)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for pixel data\n");
        free(dst->colorTab);
        free(dst->infoHeader);
        dst->colorTab  = NULL;
        dst->infoHeader = NULL;
        return -1;
    }
    memcpy(dst->pixel, src->pixel, pixelSize);

    dst->width    = src->width;
    dst->height   = src->height;
    dst->bitDepth = src->bitDepth;
    dst->offset   = src->offset;

    return 0;
}

/* GRAYSCALE CONVERSION */

/*
 * converts a 24 bpp or 32 bpp image to grayscale in-place using the
 * ITU-R BT.601 luminance formula:
 *   Y = 0.299*R + 0.587*G + 0.114*B
 *
 * for 8 bpp (palette) images the color table entries are greyed instead
 * of touching pixel indices, so the pixel data remains valid.
 */
int filterGrayscale(const BMPImage_t *src, BMPImage_t *dst)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width     = dst->width;
    int height    = abs(dst->height);
    int rowSize   = ((width * dst->bitDepth + 31) / 32) * 4;

    if (dst->bitDepth == BMP_SUPPORTED_BPP_24 || dst->bitDepth == BMP_SUPPORTED_BPP_32)
    {
        int bytesPerPixel = dst->bitDepth / 8;

        for (int y = 0; y < height; y++)
        {
            unsigned char *row = dst->pixel + y * rowSize;
            for (int x = 0; x < width; x++)
            {
                unsigned char *px = row + x * bytesPerPixel;
                /* BMP stores channels as B, G, R [, A] */
                unsigned char b = px[0];
                unsigned char g = px[1];
                unsigned char r = px[2];

                /* luminance (integer arithmetic, scaled by 1000) */
                unsigned char grey = (unsigned char)(
                    (299u * r + 587u * g + 114u * b) / 1000u);

                px[0] = grey; /* B */
                px[1] = grey; /* G */
                px[2] = grey; /* R */
                /* Alpha channel (px[3]) is preserved unchanged for 32 bpp */
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        /*
         * 8 bpp uses a palette. Each palette entry is 4 bytes: B, G, R, reserved.
         * Grey the palette entries; pixel indices stay untouched.
         */
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *entry = dst->colorTab + i * 4;
            unsigned char b = entry[0];
            unsigned char g = entry[1];
            unsigned char r = entry[2];
            unsigned char grey = (unsigned char)(
                (299u * r + 587u * g + 114u * b) / 1000u);
            entry[0] = grey;
            entry[1] = grey;
            entry[2] = grey;
            /* entry[3] (reserved/alpha) is left unchanged */
        }
    }

    return 0;
}

/* NEGATIVE CONVERSION */

/*
 * Inverts every color channel: channel = 255 - channel.
 * For 32 bpp the alpha channel is preserved.
 * For 8 bpp the palette entries are inverted instead of pixel indices.
 */
int filterNegative(const BMPImage_t *src, BMPImage_t *dst)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width  = dst->width;
    int height = abs(dst->height);
    int rowSize = ((width * dst->bitDepth + 31) / 32) * 4;

    if (dst->bitDepth == BMP_SUPPORTED_BPP_24 || dst->bitDepth == BMP_SUPPORTED_BPP_32)
    {
        int bytesPerPixel = dst->bitDepth / 8;

        for (int y = 0; y < height; y++)
        {
            unsigned char *row = dst->pixel + y * rowSize;
            for (int x = 0; x < width; x++)
            {
                unsigned char *px = row + x * bytesPerPixel;
                px[0] = 255 - px[0]; /* B */
                px[1] = 255 - px[1]; /* G */
                px[2] = 255 - px[2]; /* R */
                /* Alpha channel (px[3]) preserved for 32 bpp */
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *entry = dst->colorTab + i * 4;
            entry[0] = 255 - entry[0]; /* B */
            entry[1] = 255 - entry[1]; /* G */
            entry[2] = 255 - entry[2]; /* R */
        }
    }

    return 0;
}

/* UNIFIED INTERFACE */

int applyFilter(const BMPImage_t *src, BMPImage_t *dst, FilterType_t type)
{
    switch (type)
    {
    case FILTER_GRAYSCALE:
        return filterGrayscale(src, dst);
    case FILTER_NEGATIVE:
        return filterNegative(src, dst);
    default:
        fprintf(stderr, "ERROR: Unknown filter type\n");
        return -1;
    }
}

/* UTILITY FUNCTIONS */

const char *filterTypeToString(FilterType_t type)
{
    switch (type)
    {
    case FILTER_GRAYSCALE: return "grayscale conversion";
    case FILTER_NEGATIVE:  return "negative (invert colors)";
    default:               return "unknown";
    }
}

FilterType_t parseFilterString(const char *str)
{
    if (strcmp(str, "gray")      == 0 ||
        strcmp(str, "grey")      == 0 ||
        strcmp(str, "grayscale") == 0 ||
        strcmp(str, "greyscale") == 0 ||
        strcmp(str, "gs")        == 0)
        return FILTER_GRAYSCALE;

    if (strcmp(str, "neg")      == 0 ||
        strcmp(str, "negative") == 0 ||
        strcmp(str, "invert")   == 0 ||
        strcmp(str, "inv")      == 0)
        return FILTER_NEGATIVE;

    return (FilterType_t)-1; /* Invalid */
}

/* BATCH OPERATIONS */

int filterBatch(const BMPImage_t *src, BMPImage_t *dst,
                const FilterType_t *operations, int numOps)
{
    if (numOps <= 0)
    {
        fprintf(stderr, "ERROR: No operations specified for batch filter\n");
        return -1;
    }

    BMPImage_t temp1, temp2;
    const BMPImage_t *currentSrc = src;
    BMPImage_t       *currentDst = &temp1;
    int useTemp1 = 1;

    for (int i = 0; i < numOps; i++)
    {
        memset(currentDst, 0, sizeof(BMPImage_t));

        if (applyFilter(currentSrc, currentDst, operations[i]) != 0)
        {
            if (currentSrc != src)
                bmpFree((BMPImage_t *)currentSrc);
            return -1;
        }

        printf("  Step %d: %s\n", i + 1, filterTypeToString(operations[i]));

        if (currentSrc != src)
            bmpFree((BMPImage_t *)currentSrc);

        currentSrc = currentDst;
        currentDst = useTemp1 ? &temp2 : &temp1;
        useTemp1   = !useTemp1;
    }

    memcpy(dst, currentSrc, sizeof(BMPImage_t));

    if (currentSrc == &temp1)
        memset(&temp1, 0, sizeof(BMPImage_t));
    else
        memset(&temp2, 0, sizeof(BMPImage_t));

    return 0;
}

