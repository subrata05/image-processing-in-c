#include <stdio.h>
#include <string.h>
#include "../include/bmp.h"
#include "../include/rotation.h"
#include "../include/filter.h"

#define DEFAULT_OUTPUT_FILE "output.bmp"
#define MAX_OPS 10

/* Unified operation: either a rotation or a filter */
typedef enum { OP_ROTATE, OP_FILTER } OpKind_t;

typedef struct
{
    OpKind_t kind;
    union
    {
        RotationType_t rotation;
        FilterType_t   filter;
    } value;
} Operation_t;

/* ============ HELP / LIST ============ */

void listRotationTypes(void)
{
    printf("\nAvailable rotation types:\n");
    printf("  %-20s : %s\n", "90cw, 90, right, r",   rotationTypeToString(ROTATE_90_CW));
    printf("  %-20s : %s\n", "90ccw, left, l",        rotationTypeToString(ROTATE_90_CCW));
    printf("  %-20s : %s\n", "180, flip",              rotationTypeToString(ROTATE_180));
    printf("  %-20s : %s\n", "270cw, 270",             rotationTypeToString(ROTATE_270_CW));
    printf("  %-20s : %s\n", "hflip, mirror, h",       rotationTypeToString(ROTATE_FLIP_H));
    printf("  %-20s : %s\n", "vflip, v",               rotationTypeToString(ROTATE_FLIP_V));
    printf("  %-20s : %s\n", "transpose, diag, t",     rotationTypeToString(ROTATE_FLIP_DIAG));
    printf("  %-20s : %s\n", "antidiag, at",           rotationTypeToString(ROTATE_FLIP_ANTIDIAG));
    printf("\n");
}

void listFilterTypes(void)
{
    printf("\nAvailable filter types:\n");
    printf("  %-35s : %s\n", "gray, grey, grayscale, gs",    filterTypeToString(FILTER_GRAYSCALE));
    printf("  %-35s : %s\n", "neg, negative, invert, inv",   filterTypeToString(FILTER_NEGATIVE));
    printf("  %-35s : %s\n", "sepia, sep",                   filterTypeToString(FILTER_SEPIA));
    printf("  %-35s : %s\n", "bright, brighten, bup",        filterTypeToString(FILTER_BRIGHTNESS_UP));
    printf("  %-35s : %s\n", "darken, bdown",                filterTypeToString(FILTER_BRIGHTNESS_DOWN));
    printf("  %-35s : %s\n", "contrastup, cup, contrast+",   filterTypeToString(FILTER_CONTRAST_UP));
    printf("  %-35s : %s\n", "contrastdown, cdown, contrast-",filterTypeToString(FILTER_CONTRAST_DOWN));
    printf("  %-35s : %s\n", "threshold, thresh, bw",        filterTypeToString(FILTER_THRESHOLD));
    printf("  %-35s : %s\n", "red, redboost, rb",            filterTypeToString(FILTER_RED_BOOST));
    printf("  %-35s : %s\n", "green, greenboost, gb",        filterTypeToString(FILTER_GREEN_BOOST));
    printf("  %-35s : %s\n", "blue, blueboost, bb",          filterTypeToString(FILTER_BLUE_BOOST));
    printf("  %-35s : %s\n", "warm, warmth",                 filterTypeToString(FILTER_WARM));
    printf("  %-35s : %s\n", "cool, cold",                   filterTypeToString(FILTER_COOL));
    printf("\n");
}

void printMainUsage(const char *programName)
{
    printf("BMP Image Processor\n");
    printf("Usage: %s <input_file> [options] [output_file]\n\n", programName);
    printf("Options:\n");
    printf("  --rotate <type>    : Apply a rotation (repeatable)\n");
    printf("  --filter <type>    : Apply a filter (repeatable)\n");
    printf("  --batch  <ops>     : Rotations via comma-separated list\n");
    printf("  --list             : List rotation types\n");
    printf("  --list-filters     : List filter types\n");
    printf("  -o <file>          : Specify output file\n");
    printf("  -h, --help         : Show this help\n");
    printf("\nRotation types:\n");
    printf("  90cw/90/right/r    90ccw/left/l    180/flip\n");
    printf("  270cw/270          hflip/mirror/h  vflip/v\n");
    printf("  transpose/diag/t   antidiag/at\n");
    printf("\nFilter types:\n");
    printf("  gray/grey/grayscale/gs   : Convert to grayscale\n");
    printf("  neg/negative/invert/inv  : Negative (invert colors)\n");
    printf("  sepia/sep                : Sepia tone\n");
    printf("  bright/bup               : Brightness increase\n");
    printf("  darken/bdown             : Brightness decrease\n");
    printf("  contrastup/cup           : Contrast increase\n");
    printf("  contrastdown/cdown       : Contrast decrease\n");
    printf("  threshold/thresh/bw      : Black & white threshold\n");
    printf("  red/green/blue           : Channel boost\n");
    printf("  warm/cool                : Color temperature\n");
    printf("\nExamples:\n");
    printf("  %s photo.bmp --rotate 90cw -o rotated.bmp\n", programName);
    printf("  %s photo.bmp --filter grayscale -o gray.bmp\n", programName);
    printf("  %s photo.bmp --filter negative -o neg.bmp\n", programName);
    printf("  %s photo.bmp --filter sepia -o sepia.bmp\n", programName);
    printf("  %s photo.bmp --filter warm -o warm.bmp\n", programName);
    printf("  %s photo.bmp --filter bw -o bw.bmp\n", programName);
    printf("  %s photo.bmp --rotate 90cw --filter gray -o result.bmp\n", programName);
    printf("  %s photo.bmp --batch 90cw,hflip -o complex.bmp\n", programName);
}

/* ============ APPLY A SEQUENCE OF MIXED OPERATIONS ============ */

static int applyOperations(const BMPImage_t *src, BMPImage_t *dst,
                            const Operation_t *ops, int numOps)
{
    BMPImage_t temp1, temp2;
    const BMPImage_t *currentSrc = src;
    BMPImage_t       *currentDst = &temp1;
    int useTemp1 = 1;

    for (int i = 0; i < numOps; i++)
    {
        memset(currentDst, 0, sizeof(BMPImage_t));

        int rc = 0;
        if (ops[i].kind == OP_ROTATE)
        {
            printf("  Step %d: %s\n", i + 1,
                   rotationTypeToString(ops[i].value.rotation));
            rc = rotateImage(currentSrc, currentDst, ops[i].value.rotation);
        }
        else
        {
            printf("  Step %d: %s\n", i + 1,
                   filterTypeToString(ops[i].value.filter));
            rc = applyFilter(currentSrc, currentDst, ops[i].value.filter);
        }

        if (rc != 0)
        {
            if (currentSrc != src)
                bmpFree((BMPImage_t *)currentSrc);
            return -1;
        }

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

/* ============ MAIN ============ */

int main(int argc, char *argv[])
{
    const char  *inputFile  = NULL;
    const char  *outputFile = DEFAULT_OUTPUT_FILE;
    Operation_t  ops[MAX_OPS];
    int          numOps  = 0;
    int          useBatch = 0;
    char         batchStr[256] = {0};

    /* ---- Argument parsing ---- */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--list") == 0)
        {
            listRotationTypes();
            return 0;
        }
        else if (strcmp(argv[i], "--list-filters") == 0)
        {
            listFilterTypes();
            return 0;
        }
        else if ((strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0))
        {
            printMainUsage(argv[0]);
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
            if (numOps >= MAX_OPS)
            {
                fprintf(stderr, "ERROR: Too many operations (max %d)\n", MAX_OPS);
                return -1;
            }
            ops[numOps].kind           = OP_ROTATE;
            ops[numOps].value.rotation = type;
            numOps++;
        }
        else if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc)
        {
            FilterType_t type = parseFilterString(argv[++i]);
            if ((int)type < 0)
            {
                fprintf(stderr, "ERROR: Unknown filter type '%s'\n", argv[i]);
                fprintf(stderr, "Use --list-filters to see available types\n");
                return -1;
            }
            if (numOps >= MAX_OPS)
            {
                fprintf(stderr, "ERROR: Too many operations (max %d)\n", MAX_OPS);
                return -1;
            }
            ops[numOps].kind         = OP_FILTER;
            ops[numOps].value.filter = type;
            numOps++;
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

    /* ---- Parse --batch string (rotations only, for backward compatibility) ---- */
    if (useBatch)
    {
        char tmp[256];
        strncpy(tmp, batchStr, 255);
        char *token = strtok(tmp, ",");
        while (token != NULL && numOps < MAX_OPS)
        {
            RotationType_t type = parseRotationString(token);
            if ((int)type < 0)
            {
                fprintf(stderr, "ERROR: Unknown rotation type '%s' in batch\n", token);
                return -1;
            }
            ops[numOps].kind           = OP_ROTATE;
            ops[numOps].value.rotation = type;
            numOps++;
            token = strtok(NULL, ",");
        }
    }

    /* ---- Load ---- */
    BMPImage_t srcImg, dstImg;
    printf("Loading: %s\n", inputFile);
    if (bmpLoad(inputFile, &srcImg) != 0)
        return -1;

    printf("Loaded: %dx%d, %d bits/pixel\n",
           srcImg.width, srcImg.height, srcImg.bitDepth);

    /* ---- Process ---- */
    if (numOps == 0)
    {
        printf("No operations specified, copying only...\n");
        if (bmpSave(outputFile, &srcImg) != 0)
        {
            bmpFree(&srcImg);
            return -1;
        }
    }
    else
    {
        if (numOps == 1)
        {
            /* Single operation — fast path */
            if (ops[0].kind == OP_ROTATE)
            {
                printf("Applying: %s\n", rotationTypeToString(ops[0].value.rotation));
                if (rotateImage(&srcImg, &dstImg, ops[0].value.rotation) != 0)
                {
                    bmpFree(&srcImg);
                    return -1;
                }
            }
            else
            {
                printf("Applying: %s\n", filterTypeToString(ops[0].value.filter));
                if (applyFilter(&srcImg, &dstImg, ops[0].value.filter) != 0)
                {
                    bmpFree(&srcImg);
                    return -1;
                }
            }
        }
        else
        {
            printf("Applying %d operations:\n", numOps);
            if (applyOperations(&srcImg, &dstImg, ops, numOps) != 0)
            {
                bmpFree(&srcImg);
                return -1;
            }
        }

        printf("Result: %dx%d\n", dstImg.width, dstImg.height);

        if (bmpSave(outputFile, &dstImg) != 0)
        {
            bmpFree(&srcImg);
            bmpFree(&dstImg);
            return -1;
        }
        bmpFree(&dstImg);
    }

    bmpFree(&srcImg);
    printf("Done!\n");
    return 0;
}