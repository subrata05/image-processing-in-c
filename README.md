# BMP Image Processor

A lightweight command-line tool written in C for loading, transforming, and saving BMP image files. Supports a wide range of rotations and flips, including batch operations. More features to come later.

---

## Project Structure

```
image-processing-in-c/
├── src/
│   ├── main.c          # Entry point, argument parsing, program flow
│   ├── bmp.c           # BMP file loading, saving, and memory management
│   └── rotation.c      # All rotation and flip transformations
├── include/
│   ├── bmp.h           # BMPImage_t struct and bmp function prototypes
│   └── rotation.h      # RotationType_t enum and rotation function prototypes
├── bmp_img/            # Input BMP files here
├── .gdbinit            # GDB configuration for debugging
├── Makefile
└── README.md
```

---

## Building

### Release build (optimized)
```bash
make
# or explicitly:
make release
```

### Debug build (for GDB / Valgrind)
```bash
make debug
```

---

## Usage

```bash
./img-processor <input_file> [options] [output_file]
```

### Options

| Option | Description |
|--------|-------------|
| `--rotate <type>` | Apply a single rotation (can be used multiple times) |
| `--batch <types>` | Apply multiple rotations in one go (comma-separated) |
| `-o <file>` | Specify output file (default: `output.bmp`) |
| `--list` | List all available rotation types |
| `--help` / `-h` | Show binary usage (CLI options only, not Make targets) |

---

## Getting Help

There are two separate help systems depending on what you need:

### Binary help (CLI options)
```bash
./img-processor --help
# or
./img-processor -h
```
Shows usage for the binary itself — input/output arguments, `--rotate`, `--batch`, `-o`, and `--list`.

### Makefile help (Make targets & variables)
```bash
make help
```
Shows all Make targets, variables, and examples for the full `make run` / `make gdb` / `make valgrind` workflow.

### List rotation types
```bash
./img-processor --list
# or
make list-rotations
```

---

## Rotation Types

| Argument(s) | Effect |
|-------------|--------|
| `90cw`, `90`, `right`, `r` | 90° clockwise |
| `90ccw`, `left`, `l` | 90° counter-clockwise |
| `180`, `flip` | 180° rotation |
| `270cw`, `270` | 270° clockwise |
| `hflip`, `mirror`, `h` | Horizontal flip (mirror) |
| `vflip`, `v` | Vertical flip |
| `transpose`, `diag`, `t` | Flip along main diagonal |
| `antidiag`, `at` | Flip along anti-diagonal |

---

## Examples

```bash
# Simple 90° clockwise rotation
./img-processor bmp_img/photo.bmp --rotate 90cw -o bmp_img/rotated.bmp

# Mirror the image
./img-processor bmp_img/photo.bmp --rotate mirror -o bmp_img/mirrored.bmp

# Chain multiple rotations (mirror then rotate 90° CW)
./img-processor bmp_img/photo.bmp --rotate mirror --rotate 90cw -o bmp_img/result.bmp

# Batch rotation (comma-separated, same result as above)
./img-processor bmp_img/photo.bmp --batch mirror,90cw -o bmp_img/result.bmp

# Copy without any transformation
./img-processor bmp_img/photo.bmp -o bmp_img/copy.bmp
```

---

## Running with Make

The Makefile includes convenience targets that build and run in one step.

```bash
# Basic run
make run INPUT_FILE=photo.bmp

# Run with rotation
make run INPUT_FILE=photo.bmp ROTATION=90cw OUTPUT_FILE=bmp_img/rotated.bmp

# Batch rotation
make run INPUT_FILE=photo.bmp ROTATION=90cw,hflip OUTPUT_FILE=bmp_img/complex.bmp

# Debug with GDB
make gdb INPUT_FILE=photo.bmp ROTATION=180

# Check for memory leaks with Valgrind
make valgrind INPUT_FILE=photo.bmp ROTATION=90ccw OUTPUT_FILE=bmp_img/result.bmp
```

### Make Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `INPUT_FILE` | *(required)* | Input BMP file. If no path given, assumes `bmp_img/` |
| `OUTPUT_FILE` | `bmp_img/output.bmp` | Output BMP file |
| `ROTATION` | *(none)* | Rotation type or comma-separated batch |

---

## Supported BMP Formats

- Any BMP with a standard `BITMAPINFOHEADER` (40-byte info header or larger)
- Any bit depth: 1, 4, 8, 16, 24, 32 bpp
- Optional color table (for indexed color images)
- Both positive and negative height values (top-down and bottom-up DIBs)

---

## How It Works

### BMP Loading (`bmp.c`)
Reads the 14-byte file header and variable-length info header, extracts width, height, bit depth, and pixel data offset, then loads the optional color table and raw pixel buffer into a `BMPImage_t` struct.

### Transformations (`rotation.c`)
Each transformation maps source pixel coordinates `(x, y)` to destination coordinates `(dstX, dstY)` using a small transform function passed to a generic `allocateAndTransformPixels()` helper. Headers are rebuilt with updated width/height for operations that swap image dimensions (e.g. 90° rotations).

| Transformation | Coordinate mapping |
|----------------|--------------------|
| 90° CW | `(x, y) → (H-1-y, x)` |
| 90° CCW | `(x, y) → (y, W-1-x)` |
| 180° | `(x, y) → (W-1-x, H-1-y)` |
| Horizontal flip | `(x, y) → (W-1-x, y)` |
| Vertical flip | `(x, y) → (x, H-1-y)` |
| Transpose | `(x, y) → (y, x)` |
| Anti-diagonal | `(x, y) → (H-1-y, W-1-x)` |

### Batch Operations
`rotateBatch()` applies a sequence of transformations by ping-ponging between two temporary `BMPImage_t` buffers, freeing intermediate results as it goes.

---

## Cleaning Up

```bash
make clean
```

Removes the compiled binary and any `.o` files. BMP images in `bmp_img/` are left untouched.

---

## Dependencies

- GCC (C99 or later)
- Standard C library (`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`)
- Math library (`-lm`, linked automatically by the Makefile)
- Optional: `gdb`, `valgrind` for debugging targets