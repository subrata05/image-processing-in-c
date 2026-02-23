# BMP Image Processor

A modular C program to load, manipulate, and save BMP image files with comprehensive rotation and transformation capabilities.

---

## Features

- **Load & Save**: Read and write 24-bit and 32-bit BMP files
- **Rotation**: 90°, 180°, 270° in both directions
- **Flipping**: Horizontal mirror, vertical flip
- **Transposition**: Flip along main or anti-diagonal
- **Batch Processing**: Chain multiple transformations in one command
- **Debugging**: Full GDB support with custom inspection commands
- **Memory Safe**: Valgrind-clean with no leaks

---

## Build

```bash
make              # Build release version (optimized)
make debug        # Build debug version (for GDB)
make clean        # Clean build artifacts
```

## Basic Usage (Copy Only)

```bash
./img-processor <input.bmp> [output.bmp]
```

Examples:
```bash
./img-processor photo.bmp              # Creates output.bmp
./img-processor photo.bmp copy.bmp     # Creates copy.bmp
```

## Rotation & Transformation

### Single Operation

```bash
./img-processor input.bmp output.bmp --rotate <type>
```

Available rotation types:

| Type | Aliases | Description |
|------|---------|-------------|
| `90cw` | `90`, `right`, `r` | 90° clockwise |
| `90ccw` | `left`, `l` | 90° counter-clockwise |
| `180` | `flip` | 180° rotation |
| `270cw` | `270` | 270° clockwise |
| `hflip` | `mirror`, `h` | Horizontal flip (mirror) |
| `vflip` | `v` | Vertical flip |
| `transpose` | `diag`, `t` | Flip along main diagonal |
| `antidiag` | `at` | Flip along anti-diagonal |

Examples:
```bash
./img-processor photo.bmp rotated.bmp --rotate 90cw
./img-processor photo.bmp mirror.bmp --rotate hflip
./img-processor photo.bmp flipped.bmp --rotate 180
```

### Batch Operations (Multiple Transformations)

Chain multiple operations with comma separation:

```bash
./img-processor input.bmp output.bmp --batch <type1,type2,type3>
```

Examples:
```bash
# Mirror then rotate 90° clockwise
./img-processor photo.bmp effect.bmp --batch mirror,90cw

# Complex: rotate, flip, rotate back
./img-processor photo.bmp complex.bmp --batch 90cw,hflip,90ccw

# 180° via double 90°
./img-processor photo.bmp out.bmp --batch 90cw,90cw
```

### List Available Types

```bash
./img-processor --list
# or
make list-rotations
```

## Debug with GDB

### Using Makefile (Recommended)

```bash
make gdb INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp] [ROTATION=type]
```

Examples:
```bash
make gdb INPUT_FILE=photo.bmp
make gdb INPUT_FILE=photo.bmp ROTATION=90cw
make gdb INPUT_FILE=photo.bmp OUTPUT_FILE=rot.bmp ROTATION=hflip,90cw
```

### Manual GDB

```bash
gdb -x .gdbinit --args ./img-processor your.bmp out.bmp --rotate 90cw
```

### GDB Commands

#### Basic Commands

| Command | Description |
|---------|-------------|
| `run` | Start program |
| `continue` / `c` | Continue execution |
| `next` / `n` | Step to next line (step over) |
| `step` / `s` | Step into function |
| `finish` | Run until current function returns |
| `quit` | Exit GDB |

#### BMP Inspection Commands

| Command | Description |
|---------|-------------|
| `bmp_info` | Show image metadata |
| `bmp_header_dump` | Show raw header bytes |
| `bmp_pixel_sample` | Show pixel data sample |

#### Rotation Debugging Commands

| Command | Description |
|---------|-------------|
| `rotation_info` | List all rotation types and enum values |
| `rotation_check <n>` | Decode rotation type from integer |
| `compare_images` | Compare source vs destination properties |
| `check_dimensions` | Analyze dimension changes |

## Memory Check with Valgrind

```bash
make valgrind INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp] [ROTATION=type]
```

Examples:
```bash
make valgrind INPUT_FILE=photo.bmp
make valgrind INPUT_FILE=photo.bmp ROTATION=90ccw
make valgrind INPUT_FILE=big.bmp OUTPUT_FILE=result.bmp ROTATION=180
```

## Quick Run with Makefile

```bash
# Copy only
make run INPUT_FILE=photo.bmp

# With rotation
make run INPUT_FILE=photo.bmp ROTATION=90cw

# With custom output
make run INPUT_FILE=photo.bmp OUTPUT_FILE=rotated.bmp ROTATION=90cw

# Batch rotation
make run INPUT_FILE=photo.bmp ROTATION=hflip,90cw OUTPUT_FILE=effect.bmp
```

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make` / `make release` | Optimized build (`-O2`) |
| `make debug` | Debug build with symbols (`-g -O0`) |
| `make run` | Build and run (requires `INPUT_FILE`) |
| `make gdb` | Build and launch GDB |
| `make valgrind` | Run with memory leak detection |
| `make list-rotations` | Display available rotation types |
| `make clean` | Remove executable and GDB history |
| `make help` | Show comprehensive usage information |

## Project Structure

```
.
├── main.c           # Command-line interface and argument parsing
├── bmp.h            # BMP file format structures and I/O functions
├── bmp.c            # BMP loading, saving, and memory management
├── rotation.h       # Rotation types and transformation API
├── rotation.c       # Rotation implementations and batch processing
├── Makefile         # Build automation with input validation
├── .gdbinit         # GDB configuration with custom commands
└── README.md        # This file
```

## Technical Details

### Rotation Mathematics

All transformations use coordinate mapping formulas:

- **90° CW**: `(x, y) → (height-1-y, x)`
- **90° CCW**: `(x, y) → (y, width-1-x)`
- **180°**: `(x, y) → (width-1-x, height-1-y)`
- **H-Flip**: `(x, y) → (width-1-x, y)`
- **V-Flip**: `(x, y) → (x, height-1-y)`
- **Transpose**: `(x, y) → (y, x)`

### Memory Management

- `bmpLoad()`: Allocates `infoHeader`, `colorTab`, `pixel` → caller frees with `bmpFree()`
- `rotateImage()`: Allocates new `BMPImage_t` → caller frees with `bmpFree()`
- `rotateBatch()`: Manages internal buffers, copies result → caller frees result with `bmpFree()`

### BMP Format Support

- 24-bit RGB (most common)
- 32-bit RGBA (with alpha)
- 8-bit indexed (color table preserved through rotations)
- Bottom-up and top-down row ordering (negative height values)

## Safety Notes

- Input files are **NEVER** deleted by any `make` target
- Clean target only removes: executable, GDB history, object files
- Output files (`.bmp`) are preserved even during clean
- All file paths are relative to current working directory
- Memory leaks are prevented even on error paths

## Examples Summary

```bash
# Build and basic copy
make run INPUT_FILE=photo.bmp

# Single rotation
make run INPUT_FILE=photo.bmp ROTATION=90cw OUTPUT_FILE=rotated.bmp

# Horizontal mirror
./img-processor photo.bmp mirror.bmp --rotate hflip

# Batch: mirror then rotate
./img-processor photo.bmp out.bmp --batch mirror,90cw

# Debug rotation step-by-step
make gdb INPUT_FILE=photo.bmp ROTATION=90cw

# Check for memory leaks
make valgrind INPUT_FILE=photo.bmp ROTATION=180

# List all rotation options
make list-rotations
```