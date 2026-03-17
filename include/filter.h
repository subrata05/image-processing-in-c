#ifndef FILTER_H
#define FILTER_H

#include "bmp.h"

/* Filter types */
typedef enum
{
    FILTER_GRAYSCALE,       /* Convert to grayscale using luminance formula */
    FILTER_NEGATIVE,        /* Invert all color channels */

    /* --- New color filters --- */
    FILTER_SEPIA,           /* Warm brownish sepia tone */
    FILTER_BRIGHTNESS_UP,   /* Increase brightness by ~30% */
    FILTER_BRIGHTNESS_DOWN, /* Decrease brightness by ~30% */
    FILTER_CONTRAST_UP,     /* Increase contrast (stretch around midpoint) */
    FILTER_CONTRAST_DOWN,   /* Decrease contrast (compress around midpoint) */
    FILTER_THRESHOLD,       /* Pure black/white threshold at mid-luminance */
    FILTER_RED_BOOST,       /* Amplify red channel, mute others */
    FILTER_GREEN_BOOST,     /* Amplify green channel, mute others */
    FILTER_BLUE_BOOST,      /* Amplify blue channel, mute others */
    FILTER_WARM,            /* Warm color temperature (boost R, reduce B) */
    FILTER_COOL             /* Cool color temperature (boost B, reduce R) */
} FilterType_t;

/* Function prototypes */

/* Main filter dispatch - handles all filter types */
int applyFilter(const BMPImage_t *src, BMPImage_t *dst, FilterType_t type);

/* Individual filter functions (for direct use) */
int filterGrayscale(const BMPImage_t *src, BMPImage_t *dst);
int filterNegative(const BMPImage_t *src, BMPImage_t *dst);

/* New color filter functions */
int filterSepia(const BMPImage_t *src, BMPImage_t *dst);
int filterBrightness(const BMPImage_t *src, BMPImage_t *dst, int delta);
int filterContrast(const BMPImage_t *src, BMPImage_t *dst, float factor);
int filterThreshold(const BMPImage_t *src, BMPImage_t *dst);
int filterChannelBoost(const BMPImage_t *src, BMPImage_t *dst, int channel); /* 0=B,1=G,2=R */
int filterColorTemp(const BMPImage_t *src, BMPImage_t *dst, int rDelta, int bDelta);

/* Utility functions */
const char *filterTypeToString(FilterType_t type);
FilterType_t parseFilterString(const char *str);

/* Batch filter - multiple operations in sequence */
int filterBatch(const BMPImage_t *src, BMPImage_t *dst,
                const FilterType_t *operations, int numOps);

#endif