# BMP Loader

A modular C program to load, display, and reconstruct BMP image files.

---

## Build

```bash
make              # Build release version (optimized)
make debug        # Build debug version (for GDB)
```

## Usage

```bash
./bmp_reconstruct <input.bmp> [output.bmp]
```

Examples:

```bash
./bmp_reconstruct photo.bmp              # Creates output.bmp
./bmp_reconstruct photo.bmp copy.bmp     # Creates copy.bmp
```

## Debug with GDB

```bash
make gdb INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp]
```

Or manually:

```bash
gdb -x .gdbinit --args ./bmp_reconstruct your.bmp out.bmp
```

GDB commands:

| Command             | Description              |
|---------------------|--------------------------|
| `run`               | Start program            |
| `bmp_info`          | Show image metadata      |
| `bmp_header_dump`   | Show raw header bytes    |
| `bmp_pixel_sample`  | Show pixel data          |
| `n` / `next`        | Step to next line        |
| `quit`              | Exit GDB                 |

## Memory Check

```bash
make valgrind INPUT_FILE=your.bmp
```

## Clean

```bash
make clean        # Removes executable and GDB history
```

## Makefile Targets

| Target                  | Description                  |
|-------------------------|------------------------------|
| `make` / `make release` | Optimized build              |
| `make debug`            | Build with debug symbols     |
| `make run`              | Build and run (requires INPUT_FILE) |
| `make gdb`              | Build and debug with GDB     |
| `make valgrind`         | Check for memory leaks       |
| `make clean`            | Remove generated files       |
| `make help`             | Show usage information       |