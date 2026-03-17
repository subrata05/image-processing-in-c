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

/* ============================================================
 *  NEW COLOR FILTERS
 * ============================================================ */

/* Helper: clamp an int to [0, 255] */
static unsigned char clampByte(int v)
{
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

/* SEPIA TONE
 *
 * Uses the standard sepia matrix:
 *   outR = 0.393*R + 0.769*G + 0.189*B
 *   outG = 0.349*R + 0.686*G + 0.168*B
 *   outB = 0.272*R + 0.534*G + 0.131*B
 *
 * For 8 bpp the palette entries are transformed; pixel indices stay intact.
 */
int filterSepia(const BMPImage_t *src, BMPImage_t *dst)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                unsigned char b = px[0], g = px[1], r = px[2];

                px[2] = clampByte((int)(393*r + 769*g + 189*b) / 1000);
                px[1] = clampByte((int)(349*r + 686*g + 168*b) / 1000);
                px[0] = clampByte((int)(272*r + 534*g + 131*b) / 1000);
                /* alpha preserved for 32 bpp */
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            unsigned char b = e[0], g = e[1], r = e[2];
            e[2] = clampByte((int)(393*r + 769*g + 189*b) / 1000);
            e[1] = clampByte((int)(349*r + 686*g + 168*b) / 1000);
            e[0] = clampByte((int)(272*r + 534*g + 131*b) / 1000);
        }
    }

    return 0;
}

/* BRIGHTNESS
 *
 * Adds `delta` to every channel (clamped to [0,255]).
 * Positive delta brightens; negative delta darkens.
 */
int filterBrightness(const BMPImage_t *src, BMPImage_t *dst, int delta)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                px[0] = clampByte((int)px[0] + delta); /* B */
                px[1] = clampByte((int)px[1] + delta); /* G */
                px[2] = clampByte((int)px[2] + delta); /* R */
                /* alpha preserved for 32 bpp */
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            e[0] = clampByte((int)e[0] + delta);
            e[1] = clampByte((int)e[1] + delta);
            e[2] = clampByte((int)e[2] + delta);
        }
    }

    return 0;
}

/* CONTRAST
 *
 * Scales each channel around the midpoint (128) by `factor`:
 *   out = clamp(128 + factor * (in - 128))
 *
 * factor > 1.0 increases contrast; 0 < factor < 1.0 decreases it.
 */
int filterContrast(const BMPImage_t *src, BMPImage_t *dst, float factor)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                px[0] = clampByte((int)(128.0f + factor * ((int)px[0] - 128)));
                px[1] = clampByte((int)(128.0f + factor * ((int)px[1] - 128)));
                px[2] = clampByte((int)(128.0f + factor * ((int)px[2] - 128)));
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            e[0] = clampByte((int)(128.0f + factor * ((int)e[0] - 128)));
            e[1] = clampByte((int)(128.0f + factor * ((int)e[1] - 128)));
            e[2] = clampByte((int)(128.0f + factor * ((int)e[2] - 128)));
        }
    }

    return 0;
}

/* THRESHOLD (black & white)
 *
 * Computes luminance with ITU-R BT.601. Pixels at or above 128 become
 * white (255,255,255); below 128 become black (0,0,0).
 * For 8 bpp, palette entries are thresholded instead of pixel indices.
 */
int filterThreshold(const BMPImage_t *src, BMPImage_t *dst)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                unsigned int lum = (299u * px[2] + 587u * px[1] + 114u * px[0]) / 1000u;
                unsigned char val = (lum >= 128) ? 255 : 0;
                px[0] = val;
                px[1] = val;
                px[2] = val;
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            unsigned int lum = (299u * e[2] + 587u * e[1] + 114u * e[0]) / 1000u;
            unsigned char val = (lum >= 128) ? 255 : 0;
            e[0] = val; e[1] = val; e[2] = val;
        }
    }

    return 0;
}

/* CHANNEL BOOST
 *
 * Amplifies the selected channel (0=B, 1=G, 2=R) and halves the other two,
 * giving a strong color cast without completely zeroing channels.
 */
int filterChannelBoost(const BMPImage_t *src, BMPImage_t *dst, int channel)
{
    if (channel < 0 || channel > 2)
    {
        fprintf(stderr, "ERROR: filterChannelBoost: channel must be 0 (B), 1 (G), or 2 (R)\n");
        return -1;
    }

    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                for (int c = 0; c < 3; c++)
                {
                    if (c == channel)
                        px[c] = clampByte((int)px[c] * 3 / 2); /* +50% */
                    else
                        px[c] = px[c] / 2;                      /* -50% */
                }
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            for (int c = 0; c < 3; c++)
            {
                if (c == channel)
                    e[c] = clampByte((int)e[c] * 3 / 2);
                else
                    e[c] = e[c] / 2;
            }
        }
    }

    return 0;
}

/* COLOR TEMPERATURE (warm / cool)
 *
 * rDelta is added to the red channel; bDelta is added to the blue channel.
 * Positive rDelta / negative bDelta makes the image warmer;
 * negative rDelta / positive bDelta makes it cooler.
 */
int filterColorTemp(const BMPImage_t *src, BMPImage_t *dst, int rDelta, int bDelta)
{
    if (cloneImage(src, dst) != 0)
        return -1;

    int width   = dst->width;
    int height  = abs(dst->height);
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
                px[0] = clampByte((int)px[0] + bDelta); /* B */
                px[2] = clampByte((int)px[2] + rDelta); /* R */
                /* G and alpha unchanged */
            }
        }
    }
    else if (dst->bitDepth == BMP_SUPPORTED_BPP_8)
    {
        int numColors = dst->colorTabSize / 4;
        for (int i = 0; i < numColors; i++)
        {
            unsigned char *e = dst->colorTab + i * 4;
            e[0] = clampByte((int)e[0] + bDelta);
            e[2] = clampByte((int)e[2] + rDelta);
        }
    }

    return 0;
}

/* ============================================================ */



int applyFilter(const BMPImage_t *src, BMPImage_t *dst, FilterType_t type)
{
    switch (type)
    {
    case FILTER_GRAYSCALE:
        return filterGrayscale(src, dst);
    case FILTER_NEGATIVE:
        return filterNegative(src, dst);
    case FILTER_SEPIA:
        return filterSepia(src, dst);
    case FILTER_BRIGHTNESS_UP:
        return filterBrightness(src, dst, +77);   /* ~30% of 255 */
    case FILTER_BRIGHTNESS_DOWN:
        return filterBrightness(src, dst, -77);
    case FILTER_CONTRAST_UP:
        return filterContrast(src, dst, 1.5f);
    case FILTER_CONTRAST_DOWN:
        return filterContrast(src, dst, 0.6f);
    case FILTER_THRESHOLD:
        return filterThreshold(src, dst);
    case FILTER_RED_BOOST:
        return filterChannelBoost(src, dst, 2);   /* channel 2 = R in BMP */
    case FILTER_GREEN_BOOST:
        return filterChannelBoost(src, dst, 1);
    case FILTER_BLUE_BOOST:
        return filterChannelBoost(src, dst, 0);
    case FILTER_WARM:
        return filterColorTemp(src, dst, +40, -40);
    case FILTER_COOL:
        return filterColorTemp(src, dst, -40, +40);
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
    case FILTER_GRAYSCALE:       return "grayscale conversion";
    case FILTER_NEGATIVE:        return "negative (invert colors)";
    case FILTER_SEPIA:           return "sepia tone";
    case FILTER_BRIGHTNESS_UP:   return "brightness increase (+30%)";
    case FILTER_BRIGHTNESS_DOWN: return "brightness decrease (-30%)";
    case FILTER_CONTRAST_UP:     return "contrast increase (x1.5)";
    case FILTER_CONTRAST_DOWN:   return "contrast decrease (x0.6)";
    case FILTER_THRESHOLD:       return "threshold (black & white)";
    case FILTER_RED_BOOST:       return "red channel boost";
    case FILTER_GREEN_BOOST:     return "green channel boost";
    case FILTER_BLUE_BOOST:      return "blue channel boost";
    case FILTER_WARM:            return "warm color temperature";
    case FILTER_COOL:            return "cool color temperature";
    default:                     return "unknown";
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

    if (strcmp(str, "sepia") == 0 ||
        strcmp(str, "sep")   == 0)
        return FILTER_SEPIA;

    if (strcmp(str, "bright")    == 0 ||
        strcmp(str, "brighten")  == 0 ||
        strcmp(str, "brightup")  == 0 ||
        strcmp(str, "bup")       == 0)
        return FILTER_BRIGHTNESS_UP;

    if (strcmp(str, "darken")    == 0 ||
        strcmp(str, "darkdown")  == 0 ||
        strcmp(str, "brightdown")== 0 ||
        strcmp(str, "bdown")     == 0)
        return FILTER_BRIGHTNESS_DOWN;

    if (strcmp(str, "contrastup")   == 0 ||
        strcmp(str, "cup")          == 0 ||
        strcmp(str, "contrast+")    == 0)
        return FILTER_CONTRAST_UP;

    if (strcmp(str, "contrastdown") == 0 ||
        strcmp(str, "cdown")        == 0 ||
        strcmp(str, "contrast-")    == 0)
        return FILTER_CONTRAST_DOWN;

    if (strcmp(str, "threshold") == 0 ||
        strcmp(str, "thresh")    == 0 ||
        strcmp(str, "bw")        == 0)
        return FILTER_THRESHOLD;

    if (strcmp(str, "red")       == 0 ||
        strcmp(str, "redboost")  == 0 ||
        strcmp(str, "rb")        == 0)
        return FILTER_RED_BOOST;

    if (strcmp(str, "green")     == 0 ||
        strcmp(str, "greenboost")== 0 ||
        strcmp(str, "gb")        == 0)
        return FILTER_GREEN_BOOST;

    if (strcmp(str, "blue")      == 0 ||
        strcmp(str, "blueboost") == 0 ||
        strcmp(str, "bb")        == 0)
        return FILTER_BLUE_BOOST;

    if (strcmp(str, "warm")      == 0 ||
        strcmp(str, "warmth")    == 0)
        return FILTER_WARM;

    if (strcmp(str, "cool")      == 0 ||
        strcmp(str, "cold")      == 0)
        return FILTER_COOL;

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