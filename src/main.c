#include <stdio.h>
#include <string.h>
#include "../include/bmp.h"
#include "../include/rotation.h"

#define DEFAULT_OUTPUT_FILE "output.bmp"

void listRotationTypes(void)
{
    printf("\nAvailable rotation types:\n");
    printf("  %-20s : %s\n", "90cw, 90, right, r",
           rotationTypeToString(ROTATE_90_CW));
    printf("  %-20s : %s\n", "90ccw, left, l",
           rotationTypeToString(ROTATE_90_CCW));
    printf("  %-20s : %s\n", "180, flip",
           rotationTypeToString(ROTATE_180));
    printf("  %-20s : %s\n", "270cw, 270",
           rotationTypeToString(ROTATE_270_CW));
    printf("  %-20s : %s\n", "hflip, mirror, h",
           rotationTypeToString(ROTATE_FLIP_H));
    printf("  %-20s : %s\n", "vflip, v",
           rotationTypeToString(ROTATE_FLIP_V));
    printf("  %-20s : %s\n", "transpose, diag, t",
           rotationTypeToString(ROTATE_FLIP_DIAG));
    printf("  %-20s : %s\n", "antidiag, at",
           rotationTypeToString(ROTATE_FLIP_ANTIDIAG));
    printf("\n");
}

void printMainUsage(const char *programName)
{
    printf("BMP Image Rotation Tool\n");
    printf("Usage: %s <input_file> [options] [output_file]\n\n", programName);
    printf("Options:\n");
    printf("  --rotate <type>    : Apply rotation (can use multiple times)\n");
    printf("  --batch <types>    : Apply multiple rotations (comma-separated)\n");
    printf("  --list             : List available rotation types\n");
    printf("  -o <file>          : Specify output file\n");
    printf("\nRotation types:\n");
    printf("  90cw, 90, right, r     : 90 degrees clockwise\n");
    printf("  90ccw, left, l         : 90 degrees counter-clockwise\n");
    printf("  180, flip              : 180 degrees\n");
    printf("  270cw, 270             : 270 degrees clockwise\n");
    printf("  hflip, mirror, h       : Horizontal flip (mirror)\n");
    printf("  vflip, v               : Vertical flip\n");
    printf("  transpose, diag, t     : Transpose (flip main diagonal)\n");
    printf("  antidiag, at           : Transpose anti-diagonal\n");
    printf("\nExamples:\n");
    printf("  %s photo.bmp --rotate 90cw -o rotated.bmp\n", programName);
    printf("  %s photo.bmp --rotate mirror --rotate 90cw -o result.bmp\n", programName);
    printf("  %s photo.bmp --batch 90cw,hflip,90ccw -o complex.bmp\n", programName);
}

int main(int argc, char *argv[])
{
    const char *inputFile = NULL;
    const char *outputFile = DEFAULT_OUTPUT_FILE;
    RotationType_t operations[10];
    int numOps = 0;
    int useBatch = 0;
    char batchStr[256] = {0};

    /* Parse arguments */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--list") == 0)
        {
            listRotationTypes();
            return 0;
        }
        else if (strcmp(argv[i], "--rotate") == 0 && i + 1 < argc)
        {
            RotationType_t type = parseRotationString(argv[++i]);
            if ((int)type < 0)
            {
                fprintf(stderr, "ERROR: Unknown rotation type '%s'\n", argv[i]);
                fprintf(stderr, "Use --list to see available types\n");
                return -1;
            }
            if (numOps < 10)
            {
                operations[numOps++] = type;
            }
            else
            {
                fprintf(stderr, "ERROR: Too many rotation operations (max 10)\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "--batch") == 0 && i + 1 < argc)
        {
            strncpy(batchStr, argv[++i], 255);
            useBatch = 1;
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            outputFile = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printMainUsage(argv[0]);
            return 0;
        }
        else if (inputFile == NULL && argv[i][0] != '-')
        {
            inputFile = argv[i];
        }
        else if (strcmp(outputFile, DEFAULT_OUTPUT_FILE) == 0 && argv[i][0] != '-')
        {
            outputFile = argv[i];
        }
    }

    if (inputFile == NULL)
    {
        fprintf(stderr, "ERROR: No input file specified!\n\n");
        printMainUsage(argv[0]);
        return -1;
    }

    /* Parse batch string if provided */
    if (useBatch)
    {
        char *token = strtok(batchStr, ",");
        while (token != NULL && numOps < 10)
        {
            RotationType_t type = parseRotationString(token);
            if ((int)type < 0)
            {
                fprintf(stderr, "ERROR: Unknown rotation type '%s' in batch\n", token);
                return -1;
            }
            operations[numOps++] = type;
            token = strtok(NULL, ",");
        }
    }

    /* If no operations specified, default to copy only */
    if (numOps == 0)
    {
        printf("No rotation specified, copying only...\n");
    }

    BMPImage_t srcImg, dstImg;

    /* Load input image */
    printf("Loading: %s\n", inputFile);
    if (bmpLoad(inputFile, &srcImg) != 0)
    {
        return -1;
    }

    printf("Loaded: %dx%d, %d bits/pixel\n",
           srcImg.width, srcImg.height, srcImg.bitDepth);

    /* Perform rotation(s) */
    if (numOps > 0)
    {
        if (numOps == 1)
        {
            printf("Applying: %s\n", rotationTypeToString(operations[0]));
            if (rotateImage(&srcImg, &dstImg, operations[0]) != 0)
            {
                bmpFree(&srcImg);
                return -1;
            }
        }
        else
        {
            printf("Applying batch rotation (%d steps):\n", numOps);
            if (rotateBatch(&srcImg, &dstImg, operations, numOps) != 0)
            {
                bmpFree(&srcImg);
                return -1;
            }
        }
        printf("Result: %dx%d\n", dstImg.width, dstImg.height);

        /* Save rotated image */
        if (bmpSave(outputFile, &dstImg) != 0)
        {
            bmpFree(&srcImg);
            bmpFree(&dstImg);
            return -1;
        }
        bmpFree(&dstImg);
    }
    else
    {
        /* Just copy without rotation */
        if (bmpSave(outputFile, &srcImg) != 0)
        {
            bmpFree(&srcImg);
            return -1;
        }
    }

    /* Cleanup */
    bmpFree(&srcImg);
    printf("Done!\n");
    return 0;
}