# =============================================================================
# BMP Image Processor - GDB Configuration
# =============================================================================
# This file is loaded by GDB when you use: gdb -x .gdbinit --args ./program ...
# DO NOT put 'run' command here - arguments come from command line via --args

# =============================================================================
# GDB SETTINGS
# =============================================================================

set confirm off
set verbose off

set history filename .gdb_history
set history save on
set history expansion on

# =============================================================================
# PRETTY PRINTING
# =============================================================================

set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
set print sevenbit-strings off

# =============================================================================
# CUSTOM COMMANDS
# =============================================================================

define bmp_info
    printf "\n=== BMP Image Info ===\n"
    if srcImg.infoHeader
        printf "Source Image:\n"
        printf "  Info Header Size: %u\n", srcImg.infoHeaderSize
        printf "  Width: %d\n", srcImg.width
        printf "  Height: %d\n", srcImg.height
        printf "  Bit Depth: %u\n", srcImg.bitDepth
        printf "  Offset: %u\n", srcImg.offset
        printf "  Color Table Size: %d\n", srcImg.colorTabSize
        printf "  Pixel Pointer: %p\n", srcImg.pixel
    else
        printf "Source image not loaded or corrupted\n"
    end
    if dstImg.infoHeader
        printf "\nDestination Image:\n"
        printf "  Width: %d\n", dstImg.width
        printf "  Height: %d\n", dstImg.height
        printf "  Bit Depth: %u\n", dstImg.bitDepth
    end
    printf "======================\n\n"
end

document bmp_info
Display BMP image metadata from loaded structures (srcImg and dstImg).
end

define bmp_header_dump
    printf "\n=== Source File Header (14 bytes) ===\n"
    x/14xb srcImg.fileHeader
    printf "\n=== Source Info Header (%u bytes) ===\n", srcImg.infoHeaderSize
    if srcImg.infoHeader
        x/40xb srcImg.infoHeader
    end
end

document bmp_header_dump
Hex dump of BMP file and info headers.
end

define bmp_pixel_sample
    if srcImg.pixel
        printf "\n=== First 16 bytes of source pixel data ===\n"
        x/16xb srcImg.pixel
    else
        printf "No pixel data loaded\n"
    end
end

document bmp_pixel_sample
Show sample pixel data for verification.
end

define rotation_info
    printf "\n=== Available Rotation Types ===\n"
    printf "0  : ROTATE_90_CW        (90cw, 90, right, r)\n"
    printf "1  : ROTATE_90_CCW       (90ccw, left, l)\n"
    printf "2  : ROTATE_180          (180, flip)\n"
    printf "3  : ROTATE_270_CW       (270cw, 270)\n"
    printf "4  : ROTATE_270_CCW      (270ccw)\n"
    printf "5  : ROTATE_FLIP_H       (hflip, mirror, h)\n"
    printf "6  : ROTATE_FLIP_V       (vflip, v)\n"
    printf "7  : ROTATE_FLIP_DIAG    (transpose, diag, t)\n"
    printf "8  : ROTATE_FLIP_ANTIDIAG (antidiag, at)\n"
    printf "================================\n\n"
end

document rotation_info
List all available rotation types and their aliases.
end

# =============================================================================
# STARTUP SETUP
# =============================================================================

# Set breakpoint at main function
break main

# Auto-display these variables when stopped
display srcImg.width
display srcImg.height
display srcImg.bitDepth
display dstImg.width
display dstImg.height

# Startup message
printf "\n"
printf "GDB ready for BMP Image Processor debugging!\n"
printf "\n"
printf "Usage from terminal:\n"
printf "  gdb -x .gdbinit --args ./img-processor <input.bmp> [output.bmp] [--rotate type]\n"
printf "\n"
printf "Or use Makefile:\n"
printf "  make gdb INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp] [ROTATION=90cw]\n"
printf "\n"
printf "GDB commands available:\n"
printf "  run               - Start the program (arguments already set via --args)\n"
printf "  bmp_info          - Show image metadata\n"
printf "  bmp_header_dump   - Show raw header bytes\n"
printf "  bmp_pixel_sample  - Show pixel data sample\n"
printf "  rotation_info     - List available rotation types\n"
printf "  n or next         - Next line (step over)\n"
printf "  s or step         - Step into function\n"
printf "  c or continue     - Run to end\n"
printf "  quit              - Exit GDB\n"
printf "\n"
printf "Type 'run' to start execution with your specified files.\n"
printf "\n"