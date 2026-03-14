# BMP Image Processor

A command-line BMP image processing tool written in C with zero external libraries. Supports rotations, flips, transpositions, filters, and batch operations directly on raw BMP pixel data.

```
image-processing-in-c/
├── src/
│   ├── main.c          # Entry point and argument parsing
│   ├── bmp.c           # BMP load, save, and free
│   ├── rotation.c      # All transformation logic
│   └── filter.c        # All filter logic
├── include/
│   ├── bmp.h           # BMPImage_t struct and prototypes
│   ├── rotation.h      # RotationType_t enum and prototypes
│   └── filter.h        # FilterType_t enum and prototypes
├── bmp_img/            # Input BMP files here
├── .gdbinit            # Custom GDB debugging environment
└── Makefile
```

---

## Requirements

- GCC compiler
- GNU Make
- GDB (optional, for debugging)
- Valgrind (optional, for memory checking)

---

## Supported BMP Formats

| Bit depth | Supported | Notes |
|---|---|---|
| 8 bpp | ✅ | Indexed color with palette |
| 24 bpp | ✅ | True color (BGR) |
| 32 bpp | ✅ | True color with alpha (BGRA) |
| 4 bpp | ❌ | Not yet implemented |
| 1 bpp | ❌ | Not yet implemented |
| 16 bpp | ❌ | Not yet implemented |

---

## Quick Start

```bash
# Build the project
make

# Run a rotation
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw OUTPUT_FILE=bmp_img/out.bmp

# Run a filter
make run INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale OUTPUT_FILE=bmp_img/gray.bmp

# See all available options
make help
```

---

## Building

### `make` or `make release`
Builds the optimized release binary with `-O2 -DNDEBUG`. This is the default target — running `make` alone is equivalent to `make release`.

```bash
make
make release
```

### `make debug`
Builds the binary with debug symbols and no optimization (`-g -O0 -DDEBUG`). The `run`, `gdb`, and `valgrind` targets trigger this automatically — you do not need to run it manually before those targets.

```bash
make debug
```

### `make clean`
Removes the compiled binary, all `.o` object files, and the GDB history file (`.gdb_history`). Does **not** delete any `.bmp` image files in `bmp_img/`.

```bash
make clean
```

---

## Make Variables

These variables are passed alongside any make target that runs the binary.

| Variable | Required | Default | Description |
|---|---|---|---|
| `INPUT_FILE` | Yes (for run/gdb/valgrind) | — | Path to the source `.bmp` file. If no directory is given, assumes `bmp_img/` |
| `OUTPUT_FILE` | No | `bmp_img/output.bmp` | Path for the output `.bmp` file |
| `ROTATION` | No | — | Rotation type string (single) or comma-separated list (batch). See rotation types below |
| `FILTER` | No | — | Filter type string. See filter types below. Can be combined with `ROTATION` in the same command |

---

## Run Targets

### `make run`
Builds (debug) and runs the processor. If `ROTATION` contains a comma, it automatically uses `--batch`. Otherwise it uses `--rotate`. `FILTER` is passed as `--filter`.

```bash
# Copy only — no operation applied
make run INPUT_FILE=bmp_img/photo.bmp

# Single rotation, default output path
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw

# Single rotation with custom output path
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=180 OUTPUT_FILE=bmp_img/flipped.bmp

# Batch rotation — comma-separated, applied left to right
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw,hflip OUTPUT_FILE=bmp_img/result.bmp

# Apply a filter
make run INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale OUTPUT_FILE=bmp_img/gray.bmp

# Apply a filter and a rotation together
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw FILTER=grayscale OUTPUT_FILE=bmp_img/out.bmp

# Filename only — Makefile prepends bmp_img/ automatically
make run INPUT_FILE=photo.bmp ROTATION=vflip
```

### `make gdb`
Builds (debug) and launches GDB with `.gdbinit` loaded. Program arguments are passed via `--args` so GDB starts ready to go.

```bash
make gdb INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw
make gdb INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale
make gdb INPUT_FILE=bmp_img/photo.bmp ROTATION=180 OUTPUT_FILE=bmp_img/debug_out.bmp
```

### `make valgrind`
Builds (debug) and runs the program under Valgrind with full memory checking. Flags used: `--leak-check=full --show-leak-kinds=all --track-origins=yes`.

```bash
make valgrind INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw
make valgrind INPUT_FILE=bmp_img/photo.bmp FILTER=negative
make valgrind INPUT_FILE=bmp_img/photo.bmp ROTATION=hflip,90cw OUTPUT_FILE=bmp_img/out.bmp
```

### `make list-rotations`
Prints all rotation type strings and their aliases in the terminal. Quick reference without leaving the shell.

```bash
make list-rotations
```

### `make list-filters`
Prints all filter type strings and their aliases in the terminal.

```bash
make list-filters
```

### `make help`
Prints a full usage summary of all targets, variables, and example commands.

```bash
make help
```

---

## Using the Binary Directly

The `make run` target is a wrapper. The binary can also be called directly for full control over arguments.

### Synopsis
```
./img-processor <input_file> [options] [output_file]
```

### Options

| Option | Description |
|---|---|
| `--rotate <type>` | Apply a rotation. Can be repeated multiple times — applied in order |
| `--filter <type>` | Apply a filter. Can be repeated multiple times — applied in order |
| `--batch <types>` | Apply multiple rotations in one pass. Comma-separated, no spaces |
| `-o <file>` | Set the output file path. Default is `output.bmp` |
| `--list` | Print all available rotation type strings and exit |
| `--list-filters` | Print all available filter type strings and exit |
| `--help`, `-h` | Print usage information and exit |

### Examples

```bash
# Copy only
./img-processor bmp_img/photo.bmp -o bmp_img/copy.bmp

# Single rotation
./img-processor bmp_img/photo.bmp --rotate 90cw -o bmp_img/rotated.bmp

# Single filter
./img-processor bmp_img/photo.bmp --filter grayscale -o bmp_img/gray.bmp

# Single filter — negative
./img-processor bmp_img/photo.bmp --filter negative -o bmp_img/neg.bmp

# Chain multiple --rotate flags (applied in the order given)
./img-processor bmp_img/photo.bmp --rotate mirror --rotate 90cw -o bmp_img/result.bmp

# Chain rotation and filter together (applied in the order given)
./img-processor bmp_img/photo.bmp --rotate 90cw --filter grayscale -o bmp_img/result.bmp

# Filter first, then rotation
./img-processor bmp_img/photo.bmp --filter negative --rotate hflip -o bmp_img/result.bmp

# Batch — equivalent to chaining rotations, but in one flag
./img-processor bmp_img/photo.bmp --batch 90cw,hflip,90ccw -o bmp_img/complex.bmp

# List all rotation aliases
./img-processor --list

# List all filter aliases
./img-processor --list-filters

# Show help
./img-processor --help
```

---

## Rotation Types

All aliases for a type are interchangeable. Use whichever feels most natural.

| Rotation | Aliases | Dimensions after | Coordinate mapping |
|---|---|---|---|
| 90° clockwise | `90cw`, `90`, `right`, `r` | W↔H swapped | `(x,y) → (H-1-y, x)` |
| 90° counter-clockwise | `90ccw`, `left`, `l` | W↔H swapped | `(x,y) → (y, W-1-x)` |
| 180° | `180`, `flip` | unchanged | `(x,y) → (W-1-x, H-1-y)` |
| 270° clockwise | `270cw`, `270` | W↔H swapped | same as 90° CCW |
| 270° counter-clockwise | `270ccw` | W↔H swapped | same as 90° CW |
| Horizontal flip | `hflip`, `mirror`, `h` | unchanged | `(x,y) → (W-1-x, y)` |
| Vertical flip | `vflip`, `v` | unchanged | `(x,y) → (x, H-1-y)` |
| Transpose | `transpose`, `diag`, `t` | W↔H swapped | `(x,y) → (y, x)` |
| Anti-diagonal transpose | `antidiag`, `at` | W↔H swapped | `(x,y) → (H-1-y, W-1-x)` |

---

## Filter Types

All aliases for a type are interchangeable.

| Filter | Aliases | Description |
|---|---|---|
| Grayscale | `gray`, `grey`, `grayscale`, `greyscale`, `gs` | Converts to grayscale using the ITU-R BT.601 luminance formula: `Y = 0.299·R + 0.587·G + 0.114·B`. For 8 bpp images the palette entries are greyed instead of touching pixel indices. Alpha channel is preserved for 32 bpp. |
| Negative | `neg`, `negative`, `invert`, `inv` | Inverts every color channel: `channel = 255 - channel`. Alpha channel is preserved for 32 bpp. For 8 bpp the palette entries are inverted. |

---

## Batch Operations

Comma-separated rotations passed via `ROTATION` or `--batch` are applied in sequence, left to right. Up to 10 total operations (rotations and filters combined) per run.

```bash
# 90° CW, then flip horizontally
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw,hflip

# Three rotation operations
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw,hflip,90ccw

# Four rotation operations
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=180,hflip,vflip,transpose
```

When mixing rotations and filters, use the binary directly with repeated `--rotate` and `--filter` flags. They are applied in the order given on the command line:

```bash
# Rotate, then convert to grayscale
./img-processor bmp_img/photo.bmp --rotate 90cw --filter grayscale -o bmp_img/out.bmp

# Convert to negative, then rotate
./img-processor bmp_img/photo.bmp --filter negative --rotate hflip -o bmp_img/out.bmp

# Two rotations, then a filter
./img-processor bmp_img/photo.bmp --rotate 90cw --rotate hflip --filter grayscale -o bmp_img/out.bmp
```

---

## GDB Debugging

The `.gdbinit` file configures GDB with custom breakpoints and commands tailored specifically for this project.

### Start a Debug Session

```bash
make gdb INPUT_FILE=bmp_img/photo.bmp ROTATION=180
make gdb INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale
```

GDB loads, prints the startup message, and pauses. Type `run` to start execution.

---

### Breakpoints Set by .gdbinit

| Breakpoint | File and line | Fires when | What is in scope |
|---|---|---|---|
| BP 1 | `src/main.c:245` | Right after `bmpLoad()` returns | `srcImg` fully loaded |
| BP 2 | `src/main.c:292` | Right after any operation (rotation or filter) completes | `srcImg` and `dstImg` both ready |

---

### Auto-Display on Every Pause

These values print automatically every time execution stops, with no command needed:

```
srcImg.width
srcImg.height
srcImg.bitDepth
dstImg.width
dstImg.height
```

They show `0` before BP 1 fires — that is expected, the variables are not yet initialized.

---

### Custom GDB Commands

#### `show_src`
Prints the complete memory layout of the source image (`srcImg`):
- Struct address on the stack
- File header decoded (signature, file size, pixel data offset)
- Info header decoded (width, height, bit depth, bytes per pixel)
- Row size with padding, total pixel buffer size, pixel buffer heap address
- Color table info (size and address, or "not present" for true color)
- Raw hex dump of the 14-byte file header
- Raw hex dump of the first 40 bytes of the info header
- First 32 bytes of raw pixel data

```
(gdb) show_src
```

Available at: BP 1 and BP 2.

---

#### `show_dst`
Prints the complete memory layout of the destination/processed image (`dstImg`). Same fields as `show_src` but for `dstImg`.

```
(gdb) show_dst
```

Available at: BP 2 only — `dstImg` does not exist before the operation completes.

---

#### `compare`
Prints a side-by-side table comparing `srcImg` and `dstImg`:

```
Field                    Source           Destination
Width (px)               400              375          ← swapped for 90°
Height (px)              375              400
Bit depth (bpp)          24               24
Info header (bytes)      40               40
Color table (bytes)      0                0
Pixel offset             54               54
Pixel buffer addr        0x7ffff7f36010   0x7ffff7ec8010
```

```
(gdb) compare
```

Available at: BP 2 only.

---

#### `rotation_info`
Lists all nine `RotationType_t` enum values with their integer codes and string aliases. Useful when inspecting the `operations[]` array in `main()`.

```
(gdb) rotation_info
```

Available at: any time during execution.

---

#### `filter_info`
Lists all two `FilterType_t` enum values with their integer codes and string aliases.

```
(gdb) filter_info
```

Available at: any time during execution.

---

### Typical Debug Session

```bash
make gdb INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw
```

```
(gdb) run
# --- stops at BP 1 (after bmpLoad) ---

(gdb) show_src         # inspect source image in memory
                       # check: Width, Height, Bit depth, Pixel buffer address

(gdb) continue
# --- stops at BP 2 (after operation) ---

(gdb) show_dst         # inspect processed image in memory
                       # for 90° CW: Width and Height should be swapped
                       # for grayscale: dimensions unchanged, palette/pixel data modified

(gdb) compare          # side-by-side confirmation

(gdb) continue         # write output.bmp to disk and finish
```

---

## Error Messages Reference

| Message | Cause and fix |
|---|---|
| `ERROR: Cannot open input file '...'` | File path is wrong or file does not exist. Check `bmp_img/` contents |
| `ERROR: '...' is not a valid BMP file (signature mismatch)` | File is not a real BMP or is corrupted. Run `file yourfile.bmp` to check |
| `ERROR: Unsupported BMP header size (N)` | Header is smaller than 40 bytes — non-standard or malformed BMP |
| `ERROR: Unsupported bit depth N bpp` | Only 8, 24, 32 bpp supported. 1, 4, 16 bpp are not yet implemented |
| `ERROR: Unknown rotation type '...'` | Invalid string. Run `--list` or `make list-rotations` to see valid aliases |
| `ERROR: Unknown filter type '...'` | Invalid string. Run `--list-filters` or `make list-filters` to see valid aliases |
| `ERROR: Too many operations (max 10)` | More than 10 combined rotation and filter operations specified |
| `ERROR: No input file specified!` | `INPUT_FILE` was not set. Add `INPUT_FILE=bmp_img/photo.bmp` to the command |
| `ERROR: Memory allocation failed` | System ran out of memory — image may be too large |
| `ERROR: Cannot read ...` | File is truncated or unreadable mid-read. The file may be corrupted |

---

## Examples at a Glance

```bash
# Build release
make

# Rotate 90° clockwise
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw OUTPUT_FILE=bmp_img/out.bmp

# Rotate 180°
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=180 OUTPUT_FILE=bmp_img/out.bmp

# Flip horizontally
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=hflip OUTPUT_FILE=bmp_img/out.bmp

# Flip vertically
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=vflip OUTPUT_FILE=bmp_img/out.bmp

# Transpose
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=transpose OUTPUT_FILE=bmp_img/out.bmp

# Convert to grayscale
make run INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale OUTPUT_FILE=bmp_img/gray.bmp

# Convert to negative
make run INPUT_FILE=bmp_img/photo.bmp FILTER=negative OUTPUT_FILE=bmp_img/neg.bmp

# Rotate and apply grayscale together
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw FILTER=grayscale OUTPUT_FILE=bmp_img/out.bmp

# Batch: 90° CW then horizontal flip
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw,hflip OUTPUT_FILE=bmp_img/out.bmp

# Batch: three operations
make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw,hflip,90ccw OUTPUT_FILE=bmp_img/out.bmp

# Mixed pipeline via binary: rotate then filter
./img-processor bmp_img/photo.bmp --rotate 90cw --filter grayscale -o bmp_img/out.bmp

# Mixed pipeline via binary: filter then rotate
./img-processor bmp_img/photo.bmp --filter negative --rotate hflip -o bmp_img/out.bmp

# Debug in GDB with a filter
make gdb INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale

# Debug in GDB with a rotation
make gdb INPUT_FILE=bmp_img/photo.bmp ROTATION=180

# Check for memory leaks
make valgrind INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw
make valgrind INPUT_FILE=bmp_img/photo.bmp FILTER=negative

# List all rotation type aliases
make list-rotations

# List all filter type aliases
make list-filters

# See all make targets and variables
make help

# Clean build artifacts (images are NOT deleted)
make clean
```