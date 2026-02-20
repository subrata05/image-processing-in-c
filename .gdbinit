# =============================================================================
# BMP Loader - GDB Configuration
# =============================================================================
# This file is loaded by GDB when you use: gdb -x .gdbinit --args ./program ...
# DO NOT put 'run' command here - arguments come from command line via --args

# =============================================================================
# GDB SETTINGS
# =============================================================================

set confirm off
# Don't ask "Are you sure?" before operations

set verbose off
# Keep output clean

set history filename .gdb_history
set history save on
set history expansion on
# Save command history between sessions

# =============================================================================
# PRETTY PRINTING
# =============================================================================

set print pretty on
# Structures print with newlines and indentation

set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
set print sevenbit-strings off
# Various formatting improvements

# =============================================================================
# CUSTOM COMMANDS
# =============================================================================

define bmp_info
    printf "\n=== BMP Image Info ===\n"
    if img.infoHeader
        printf "Info Header Size: %u\n", img.infoHeaderSize
        printf "Width: %d\n", img.width
        printf "Height: %d\n", img.height
        printf "Bit Depth: %u\n", img.bitDepth
        printf "Offset: %u\n", img.offset
        printf "Color Table Size: %d\n", img.colorTabSize
        printf "Pixel Pointer: %p\n", img.pixel
    else
        printf "Image not loaded or corrupted\n"
    end
    printf "======================\n"
end

document bmp_info
Display BMP image metadata from loaded structure.
end

define bmp_header_dump
    printf "\n=== File Header (14 bytes) ===\n"
    x/14xb img.fileHeader
    printf "\n=== Info Header (%u bytes) ===\n", img.infoHeaderSize
    if img.infoHeader
        x/40xb img.infoHeader
    end
end

document bmp_header_dump
Hex dump of BMP file and info headers.
end

define bmp_pixel_sample
    if img.pixel
        printf "\n=== First 16 bytes of pixel data ===\n"
        x/16xb img.pixel
    else
        printf "No pixel data loaded\n"
    end
end

document bmp_pixel_sample
Show sample pixel data for verification.
end

# =============================================================================
# STARTUP SETUP
# =============================================================================

# Set breakpoint at main function
break main

# DO NOT add 'run' command here!
# The Makefile passes arguments via: gdb --args ./bmp_reconstruct input.bmp output.bmp
# Or you can run manually: gdb --args ./bmp_reconstruct your.bmp out.bmp

# Auto-display these variables when stopped
display img.width
display img.height
display img.bitDepth

# Startup message
printf "\n"
printf "GDB ready for BMP debugging!\n"
printf "\n"
printf "Usage from terminal:\n"
printf "  gdb -x .gdbinit --args ./bmp_reconstruct <input.bmp> [output.bmp]\n"
printf "\n"
printf "Or use Makefile:\n"
printf "  make gdb INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp]\n"
printf "\n"
printf "GDB commands available:\n"
printf "  run               - Start the program (arguments already set via --args)\n"
printf "  bmp_info          - Show image metadata\n"
printf "  bmp_header_dump   - Show raw header bytes\n"
printf "  bmp_pixel_sample  - Show pixel data sample\n"
printf "  n or next         - Next line (step over)\n"
printf "  s or step         - Step into function\n"
printf "  c or continue     - Run to end\n"
printf "  quit              - Exit GDB\n"
printf "\n"
printf "Type 'run' to start execution with your specified files.\n"
printf "\n"