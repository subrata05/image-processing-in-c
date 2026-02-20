#include <stdio.h>
#include "bmp.h"

#define DEFAULT_OUTPUT_FILE     "output.bmp"

int main(int argc, char *argv[]) {
    /* argc = argument count, argv = argument values */
    /* argv[0] = program name, argv[1] = first argument, etc. */
    
    const char *inputFile;
    const char *outputFile;
    
    /* Check if user provided at least input file */
    if (argc < 2) {
        fprintf(stderr, "ERROR: No input file specified!\n\n");
        printUsage(argv[0]);
        return -1;
    }
    
    /* First argument after program name is input file (required) */
    inputFile = argv[1];
    
    /* Second argument is output file (optional), use default if not provided */
    if (argc >= 3) {
        outputFile = argv[2];           /* User provided output filename */
    } else {
        outputFile = DEFAULT_OUTPUT_FILE;  /* Use default: output.bmp */
        printf("No output file specified, using default: %s\n", outputFile);
    }
    
    /* If more than 2 arguments, warn about ignoring extras */
    if (argc > 3) {
        printf("Warning: Extra arguments ignored. Only using first two.\n");
    }

    BMPImage_t img;

    /* Load input image */
    printf("Loading: %s\n", inputFile);
    if (bmpLoad(inputFile, &img) != 0) {
        return -1;
    }

    printf("Loaded: %dx%d, %d bits/pixel\n", img.width, img.height, img.bitDepth);

    /* Save to output file */
    if (bmpSave(outputFile, &img) != 0) {
        bmpFree(&img);
        return -1;
    }

    /* Cleanup */
    bmpFree(&img);
    return 0;
}