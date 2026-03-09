# =============================================================================
# BMP Image Processor - GDB Configuration
# =============================================================================

# =============================================================================
# GDB SETTINGS
# =============================================================================

set confirm off
set verbose off

set history filename .gdb_history
set history save on
set history expansion on

set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
set print sevenbit-strings off

# =============================================================================
# BREAKPOINTS
#
# Two strategic breakpoints, both inside main() where srcImg and dstImg
# are guaranteed to be in scope:
#
#   BP 1 — line 154 (src/main.c)
#          The printf right after bmpLoad() returns successfully.
#          At this point srcImg is fully loaded from disk.
#          Use: inspect the source image memory layout.
#
#   BP 2 — line 177 (src/main.c)
#          The printf("Result: %dx%d\n", ...) right after rotateImage()
#          or rotateBatch() returns.
#          At this point both srcImg AND dstImg are fully populated.
#          Use: inspect the rotated image memory layout and compare.
#
# =============================================================================

break src/main.c:154
break src/main.c:177

# =============================================================================
# CUSTOM COMMANDS
# =============================================================================

# -----------------------------------------------------------------------------
# show_src — memory layout of the SOURCE image (srcImg)
# Use after BP 1 (line 154) or BP 2 (line 177)
# -----------------------------------------------------------------------------
define show_src
    printf "\n"
    printf "---------- SOURCE IMAGE MEMORY LAYOUT (srcImg) ----------\n"
    printf "\n"
    printf "Struct address: \n"
    printf "  &srcImg              : %p\n", &srcImg
    printf "\n"
    printf "File Header (14 bytes at %p): \n", srcImg.fileHeader
    printf "  Signature            : %c%c\n",        srcImg.fileHeader[0], srcImg.fileHeader[1]
    printf "  File size (bytes)    : %u\n",          (unsigned)(srcImg.fileHeader[2]) | ((unsigned)(srcImg.fileHeader[3]) << 8) | ((unsigned)(srcImg.fileHeader[4]) << 16) | ((unsigned)(srcImg.fileHeader[5]) << 24)
    printf "  Pixel data offset    : %u\n",          srcImg.offset
    printf "\n"
    printf "Info Header (%u bytes at %p): \n", srcImg.infoHeaderSize, srcImg.infoHeader
    printf "  Width                : %d px\n",       srcImg.width
    printf "  Height               : %d px\n",       srcImg.height
    printf "  Bit depth            : %u bpp\n",      srcImg.bitDepth
    printf "  Bytes per pixel      : %u\n",          srcImg.bitDepth / 8
    printf "\n"
    printf "Row & pixel buffer: \n"
    printf "  Row size (w/ padding): %u bytes\n",    (((unsigned)(srcImg.width) * (unsigned)(srcImg.bitDepth) + 31) / 32) * 4
    printf "  Total pixel buffer   : %u bytes\n",    (((unsigned)(srcImg.width) * (unsigned)(srcImg.bitDepth) + 31) / 32) * 4 * (srcImg.height < 0 ? -srcImg.height : srcImg.height)
    printf "  Pixel buffer address : %p\n",          srcImg.pixel
    printf "\n"
    printf "Color table: \n"
    printf "  Color table size     : %d bytes\n",    srcImg.colorTabSize
    if srcImg.colorTabSize > 0
        printf "  Color table address  : %p\n",      srcImg.colorTab
    else
        printf "  Color table          : not present (true color image)\n"
    end
    printf "\n"
    printf "Raw file header hex dump: \n"
    x/14xb srcImg.fileHeader
    printf "\n"
    printf "Raw info header hex dump (first 40 bytes): \n"
    x/40xb srcImg.infoHeader
    printf "\n"
    printf "First 32 bytes of pixel data: \n"
    x/32xb srcImg.pixel
    printf "\n"
    printf "-----------------------------------------------------------\n"
    printf "\n"
end
document show_src
Show the full memory layout of the source image (srcImg).
Available after breakpoint 1 (line 154) — right after bmpLoad().
end

# -----------------------------------------------------------------------------
# show_dst — memory layout of the ROTATED/DESTINATION image (dstImg)
# Use after BP 2 (line 177) only — dstImg does not exist before rotation
# -----------------------------------------------------------------------------
define show_dst
    printf "\n"
    printf "---------- DESTINATION IMAGE MEMORY LAYOUT (dstImg) ----------\n"
    printf "\n"
    printf "Struct address: \n"
    printf "  &dstImg              : %p\n", &dstImg
    printf "\n"
    printf "File Header (14 bytes at %p): \n", dstImg.fileHeader
    printf "  Signature            : %c%c\n",        dstImg.fileHeader[0], dstImg.fileHeader[1]
    printf "  File size (bytes)    : %u\n",          (unsigned)(dstImg.fileHeader[2]) | ((unsigned)(dstImg.fileHeader[3]) << 8) | ((unsigned)(dstImg.fileHeader[4]) << 16) | ((unsigned)(dstImg.fileHeader[5]) << 24)
    printf "  Pixel data offset    : %u\n",          dstImg.offset
    printf "\n"
    printf "Info Header (%u bytes at %p): \n", dstImg.infoHeaderSize, dstImg.infoHeader
    printf "  Width                : %d px\n",       dstImg.width
    printf "  Height               : %d px\n",       dstImg.height
    printf "  Bit depth            : %u bpp\n",      dstImg.bitDepth
    printf "  Bytes per pixel      : %u\n",          dstImg.bitDepth / 8
    printf "\n"
    printf "Row & pixel buffer: \n"
    printf "  Row size (w/ padding): %u bytes\n",    (((unsigned)(dstImg.width) * (unsigned)(dstImg.bitDepth) + 31) / 32) * 4
    printf "  Total pixel buffer   : %u bytes\n",    (((unsigned)(dstImg.width) * (unsigned)(dstImg.bitDepth) + 31) / 32) * 4 * (dstImg.height < 0 ? -dstImg.height : dstImg.height)
    printf "  Pixel buffer address : %p\n",          dstImg.pixel
    printf "\n"
    printf "Color table: \n"
    printf "  Color table size     : %d bytes\n",    dstImg.colorTabSize
    if dstImg.colorTabSize > 0
        printf "  Color table address  : %p\n",      dstImg.colorTab
    else
        printf "  Color table          : not present (true color image)\n"
    end
    printf "\n"
    printf "Raw file header hex dump: \n"
    x/14xb dstImg.fileHeader
    printf "\n"
    printf "Raw info header hex dump (first 40 bytes): \n"
    x/40xb dstImg.infoHeader
    printf "\n"
    printf "First 32 bytes of pixel data: \n"
    x/32xb dstImg.pixel
    printf "\n"
    printf "-----------------------------------------------------------\n"
    printf "\n"
end
document show_dst
Show the full memory layout of the destination/rotated image (dstImg).
Available after breakpoint 2 (line 177) — right after rotation completes.
end

# -----------------------------------------------------------------------------
# compare — side by side comparison of src vs dst dimensions
# Use after BP 2 (line 177) only
# -----------------------------------------------------------------------------
define compare
    printf "\n"
    printf "  SRC vs DST COMPARISON\n"
    printf "-----------------------------------------------------------\n"
    printf "  %-24s %-16s %-16s\n", "Field", "Source", "Destination"
    printf "  %-24s %-16d %-16d\n", "Width (px)",        srcImg.width,     dstImg.width
    printf "  %-24s %-16d %-16d\n", "Height (px)",       srcImg.height,    dstImg.height
    printf "  %-24s %-16u %-16u\n", "Bit depth (bpp)",   srcImg.bitDepth,  dstImg.bitDepth
    printf "  %-24s %-16u %-16u\n", "Info header (bytes)", srcImg.infoHeaderSize, dstImg.infoHeaderSize
    printf "  %-24s %-16d %-16d\n", "Color table (bytes)", srcImg.colorTabSize,   dstImg.colorTabSize
    printf "  %-24s %-16u %-16u\n", "Pixel offset",      srcImg.offset,    dstImg.offset
    printf "  %-24s %p %p\n",       "Pixel buffer addr", srcImg.pixel,     dstImg.pixel
    printf "-----------------------------------------------------------\n"
    printf "\n"
end
document compare
Side-by-side comparison of srcImg and dstImg dimensions and layout.
Available after breakpoint 2 (line 177) — right after rotation completes.
end

# -----------------------------------------------------------------------------
# rotation_info — list all rotation types and aliases
# -----------------------------------------------------------------------------
define rotation_info
    printf "\nAvailable Rotation Types: \n"
    printf "0  : ROTATE_90_CW         (90cw, 90, right, r)\n"
    printf "1  : ROTATE_90_CCW        (90ccw, left, l)\n"
    printf "2  : ROTATE_180           (180, flip)\n"
    printf "3  : ROTATE_270_CW        (270cw, 270)\n"
    printf "4  : ROTATE_270_CCW       (270ccw)\n"
    printf "5  : ROTATE_FLIP_H        (hflip, mirror, h)\n"
    printf "6  : ROTATE_FLIP_V        (vflip, v)\n"
    printf "7  : ROTATE_FLIP_DIAG     (transpose, diag, t)\n"
    printf "8  : ROTATE_FLIP_ANTIDIAG (antidiag, at)\n"
    printf "--------------------------------\n\n"
end

document rotation_info
List all available rotation types and their string aliases.
end

# =============================================================================
# AUTO-DISPLAY — printed automatically every time execution stops
# =============================================================================

# These will show as "optimized out" or 0 until the program reaches line 144
# where srcImg and dstImg are declared. That is expected.
display srcImg.width
display srcImg.height
display srcImg.bitDepth
display dstImg.width
display dstImg.height

# =============================================================================
# STARTUP MESSAGE
# =============================================================================

printf "\n"
printf "BMP Image Processor — GDB ready\n"
printf "\n"
printf "Breakpoints set:\n"
printf "  BP 1 -> src/main.c:154  (right after bmpLoad  — srcImg is ready)\n"
printf "  BP 2 -> src/main.c:177  (right after rotation — dstImg is ready)\n"
printf "\n"
printf "Commands:\n"
printf "  show_src       Show source image full memory layout\n"
printf "  show_dst       Show destination/rotated image memory layout\n"
printf "  compare        Side-by-side src vs dst comparison\n"
printf "  rotation_info  List all rotation types and aliases\n"
printf "\n"
printf "Workflow:\n"
printf "  run            Start the program\n"
printf "  (stops at BP1) show_src       <- source image layout\n"
printf "  continue\n"
printf "  (stops at BP2) show_dst       <- rotated image layout\n"
printf "                 compare        <- src vs dst side by side\n"
printf "  continue       <- write output file and finish\n"
printf "\n"