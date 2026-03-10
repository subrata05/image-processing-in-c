#ifndef FILTER_H
#define FILTER_H

#include "bmp.h"

/* Filter types */
typedef enum
{
    FILTER_GRAYSCALE,  /* Convert to grayscale using luminance formula */
    FILTER_NEGATIVE    /* Invert all color channels */
} FilterType_t;

/* Function prototypes */

/* Main filter dispatch - handles all filter types */
int applyFilter(const BMPImage_t *src, BMPImage_t *dst, FilterType_t type);

/* Individual filter functions (for direct use) */
int filterGrayscale(const BMPImage_t *src, BMPImage_t *dst);
int filterNegative(const BMPImage_t *src, BMPImage_t *dst);

/* Utility functions */
const char *filterTypeToString(FilterType_t type);
FilterType_t parseFilterString(const char *str);

/* Batch filter - multiple operations in sequence */
int filterBatch(const BMPImage_t *src, BMPImage_t *dst,
                const FilterType_t *operations, int numOps);

#endif