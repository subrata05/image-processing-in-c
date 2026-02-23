#include "bmp.h"

/* Helper: Read little-endian values safely */
static uint32_t readU32LE(const unsigned char *data, int offset)
{
    return (uint32_t)data[offset] |
           ((uint32_t)data[offset + 1] << 8) |
           ((uint32_t)data[offset + 2] << 16) |
           ((uint32_t)data[offset + 3] << 24);
}

static uint16_t readU16LE(const unsigned char *data, int offset)
{
    return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
}

static int32_t readS32LE(const unsigned char *data, int offset)
{
    return (int32_t)readU32LE(data, offset);
}

void printUsage(const char *programName)
{
    printf("Usage: %s <input_file> [output_file]\n", programName);
    printf("  input_file   : Source BMP file to load (required)\n");
    printf("  output_file  : Destination BMP file (optional, default: output.bmp)\n");
    printf("\nExamples:\n");
    printf("  %s photo.bmp                    # Output: output.bmp\n", programName);
    printf("  %s photo.bmp copy.bmp          # Output: copy.bmp\n", programName);
}

int bmpLoad(const char *filename, BMPImage_t *img)
{
    memset(img, 0, sizeof(BMPImage_t));

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "ERROR: Cannot open input file '%s'\n", filename);
        return -1;
    }

    /* Read file header (14 bytes) */
    if (fread(img->fileHeader, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE)
    {
        fprintf(stderr, "ERROR: Cannot read file header from '%s'\n", filename);
        fclose(fp);
        return -1;
    }

    /* Verify BMP signature */
    if (img->fileHeader[0] != 'B' || img->fileHeader[1] != 'M')
    {
        fprintf(stderr, "ERROR: '%s' is not a valid BMP file (signature mismatch)\n", filename);
        fclose(fp);
        return -1;
    }

    /* Read info header size */
    unsigned char sizeBuf[4];
    if (fread(sizeBuf, 1, 4, fp) != 4)
    {
        fprintf(stderr, "ERROR: Cannot read info header size from '%s'\n", filename);
        fclose(fp);
        return -1;
    }
    img->infoHeaderSize = readU32LE(sizeBuf, 0);

    if (img->infoHeaderSize < BMP_MIN_INFO_SIZE)
    {
        fprintf(stderr, "ERROR: Unsupported BMP header size (%u) in '%s'\n", img->infoHeaderSize, filename);
        fclose(fp);
        return -1;
    }

    /* Allocate and read full info header */
    img->infoHeader = (unsigned char *)malloc(img->infoHeaderSize);
    if (!img->infoHeader)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for info header\n");
        fclose(fp);
        return -1;
    }

    memcpy(img->infoHeader, sizeBuf, 4);
    if (fread(img->infoHeader + 4, 1, img->infoHeaderSize - 4, fp) != img->infoHeaderSize - 4)
    {
        fprintf(stderr, "ERROR: Cannot read info header from '%s'\n", filename);
        free(img->infoHeader);
        img->infoHeader = NULL;
        fclose(fp);
        return -1;
    }

    /* Extract fields */
    img->offset = readU32LE(img->fileHeader, 10);
    img->width = readS32LE(img->infoHeader, 4);
    img->height = readS32LE(img->infoHeader, 8);
    img->bitDepth = readU16LE(img->infoHeader, 14);

    /* Read color table if present */
    uint32_t totalHeader = BMP_FILE_HEADER_SIZE + img->infoHeaderSize;
    img->colorTabSize = (int)(img->offset - totalHeader);

    if (img->colorTabSize > 0)
    {
        img->colorTab = (unsigned char *)malloc(img->colorTabSize);
        if (!img->colorTab)
        {
            fprintf(stderr, "ERROR: Memory allocation failed for color table\n");
            free(img->infoHeader);
            img->infoHeader = NULL;
            fclose(fp);
            return -1;
        }
        if (fread(img->colorTab, 1, img->colorTabSize, fp) != (size_t)img->colorTabSize)
        {
            fprintf(stderr, "ERROR: Cannot read color table from '%s'\n", filename);
            free(img->colorTab);
            free(img->infoHeader);
            img->colorTab = NULL;
            img->infoHeader = NULL;
            fclose(fp);
            return -1;
        }
    }

    /* Read pixel data */
    int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
    int pixelSize = rowSize * abs(img->height);

    img->pixel = (unsigned char *)malloc(pixelSize);
    if (!img->pixel)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for pixel data\n");
        free(img->colorTab);
        free(img->infoHeader);
        img->pixel = NULL;
        img->colorTab = NULL;
        img->infoHeader = NULL;
        fclose(fp);
        return -1;
    }

    fseek(fp, img->offset, SEEK_SET);
    if (fread(img->pixel, 1, pixelSize, fp) != (size_t)pixelSize)
    {
        fprintf(stderr, "ERROR: Cannot read pixel data from '%s'\n", filename);
        free(img->pixel);
        free(img->colorTab);
        free(img->infoHeader);
        img->pixel = NULL;
        img->colorTab = NULL;
        img->infoHeader = NULL;
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int bmpSave(const char *filename, const BMPImage_t *img)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "ERROR: Cannot create output file '%s'\n", filename);
        return -1;
    }

    int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
    int pixelSize = rowSize * abs(img->height);

    if (fwrite(img->fileHeader, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE ||
        fwrite(img->infoHeader, 1, img->infoHeaderSize, fp) != img->infoHeaderSize)
    {
        fprintf(stderr, "ERROR: Failed to write headers to '%s'\n", filename);
        fclose(fp);
        return -1;
    }

    if (img->colorTabSize > 0)
    {
        if (fwrite(img->colorTab, 1, img->colorTabSize, fp) != (size_t)img->colorTabSize)
        {
            fprintf(stderr, "ERROR: Failed to write color table to '%s'\n", filename);
            fclose(fp);
            return -1;
        }
    }

    if (fwrite(img->pixel, 1, pixelSize, fp) != (size_t)pixelSize)
    {
        fprintf(stderr, "ERROR: Failed to write pixel data to '%s'\n", filename);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    printf("Image saved successfully to '%s'\n", filename);
    return 0;
}

void bmpFree(BMPImage_t *img)
{
    free(img->infoHeader);
    free(img->colorTab);
    free(img->pixel);
    memset(img, 0, sizeof(BMPImage_t));
}