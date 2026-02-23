# =============================================================================
# BMP Image Processor Makefile
# =============================================================================

# =============================================================================
# COMPILER SETTINGS
# =============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99
TARGET = img-processor
SRCS = main.c bmp.c rotation.c

# =============================================================================
# USER CONFIGURABLE VARIABLES
# =============================================================================

# INPUT_FILE: REQUIRED - must be specified by user
# No default here! User MUST provide: make run INPUT_FILE=photo.bmp
INPUT_FILE ?=

# OUTPUT_FILE: Optional - defaults to output.bmp if not specified
OUTPUT_FILE ?= output.bmp

# ROTATION: Optional - rotation type (90cw, 90ccw, 180, hflip, vflip, etc.)
# Can specify multiple rotations separated by commas for batch processing
# Examples: ROTATION=90cw | ROTATION=hflip,90cw | ROTATION=180
ROTATION ?=

# =============================================================================
# BUILD TARGETS
# =============================================================================

.PHONY: all clean debug release gdb valgrind run help check-input list-rotations

# -----------------------------------------------------------------------------
# TARGET: all (default)
# -----------------------------------------------------------------------------
all: release

# -----------------------------------------------------------------------------
# TARGET: release (optimized)
# -----------------------------------------------------------------------------
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

# -----------------------------------------------------------------------------
# TARGET: debug (with symbols)
# -----------------------------------------------------------------------------
debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# -----------------------------------------------------------------------------
# TARGET: $(TARGET) (the executable)
# -----------------------------------------------------------------------------
$(TARGET): $(SRCS) bmp.h rotation.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

# =============================================================================
# INPUT VALIDATION
# =============================================================================

# This target checks if INPUT_FILE is provided
check-input:
ifeq ($(INPUT_FILE),)
	@echo "ERROR: No input file specified!"
	@echo ""
	@echo "Usage: make <target> INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp] [ROTATION=type]"
	@echo ""
	@echo "Examples:"
	@echo "  make run INPUT_FILE=photo.bmp"
	@echo "  make run INPUT_FILE=mr.bean.bmp OUTPUT_FILE=copy.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=90cw OUTPUT_FILE=rotated.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=hflip,90cw OUTPUT_FILE=complex.bmp"
	@echo "  make gdb INPUT_FILE=test.bmp ROTATION=180"
	@echo "  make valgrind INPUT_FILE=big.bmp ROTATION=90ccw OUTPUT_FILE=result.bmp"
	@echo ""
	@exit 1
endif
# @exit 1 stops the make process with error
# The @ suppresses echoing the command itself

# =============================================================================
# UTILITY TARGETS (all require input file)
# =============================================================================

# -----------------------------------------------------------------------------
# TARGET: gdb (start debugger)
# -----------------------------------------------------------------------------
gdb: debug check-input
	@echo "Starting GDB with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)"
	gdb -x .gdbinit --args ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)

# -----------------------------------------------------------------------------
# TARGET: valgrind (memory check)
# -----------------------------------------------------------------------------
valgrind: debug check-input
	@echo "Running Valgrind with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)

# -----------------------------------------------------------------------------
# TARGET: run (quick test)
# -----------------------------------------------------------------------------
# Build rotation flags: if ROTATION is set, prepend --rotate or --batch
ROTATION_FLAGS := $(if $(ROTATION),$(if $(findstring ,,$(ROTATION)),--batch $(ROTATION),--rotate $(ROTATION)),)

run: debug check-input
	@echo "Running: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)"
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(ROTATION_FLAGS)

# -----------------------------------------------------------------------------
# TARGET: list-rotations (show available rotation types)
# -----------------------------------------------------------------------------
list-rotations: debug
	@echo "Available rotation types:"
	@echo "  90cw, 90, right, r     : 90 degrees clockwise"
	@echo "  90ccw, left, l         : 90 degrees counter-clockwise"
	@echo "  180, flip              : 180 degrees"
	@echo "  270cw, 270             : 270 degrees clockwise"
	@echo "  hflip, mirror, h       : Horizontal flip (mirror)"
	@echo "  vflip, v               : Vertical flip"
	@echo "  transpose, diag, t     : Transpose (flip main diagonal)"
	@echo "  antidiag, at           : Transpose anti-diagonal"
	@echo ""
	@echo "Batch operations (comma-separated):"
	@echo "  make run INPUT_FILE=a.bmp ROTATION=90cw,hflip,90ccw OUTPUT_FILE=out.bmp"

# =============================================================================
# UTILITY TARGETS (no input required)
# =============================================================================

# -----------------------------------------------------------------------------
# TARGET: clean
# -----------------------------------------------------------------------------
clean:
	rm -f $(TARGET) .gdb_history
	@echo "Cleaned: executable and GDB history"
	@echo "Note: Output files (*.bmp) are NOT deleted - delete manually if needed"

# -----------------------------------------------------------------------------
# TARGET: help
# -----------------------------------------------------------------------------
help:
	@echo "=================================================================="
	@echo "BMP Image Processor Makefile"
	@echo "=================================================================="
	@echo ""
	@echo "BUILD TARGETS (no input file needed):"
	@echo "  make              Build release version (optimized)"
	@echo "  make release      Same as 'make'"
	@echo "  make debug        Build debug version (for GDB/Valgrind)"
	@echo "  make clean        Remove executable and GDB history"
	@echo "  make list-rotations   Show available rotation types"
	@echo ""
	@echo "RUN TARGETS (INPUT_FILE is REQUIRED):"
	@echo "  make run INPUT_FILE=your.bmp                    # Copy only"
	@echo "  make run INPUT_FILE=a.bmp ROTATION=90cw         # Rotate 90° CW"
	@echo "  make run INPUT_FILE=a.bmp ROTATION=90cw,hflip   # Batch rotation"
	@echo "  make gdb INPUT_FILE=your.bmp [ROTATION=type]    # Debug with GDB"
	@echo "  make valgrind INPUT_FILE=your.bmp [ROTATION=t]  # Check memory"
	@echo ""
	@echo "VARIABLES:"
	@echo "  INPUT_FILE   : Source BMP file (REQUIRED)"
	@echo "  OUTPUT_FILE  : Destination file (default: output.bmp)"
	@echo "  ROTATION     : Rotation type or batch (comma-separated)"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make run INPUT_FILE=mr.bean.bmp"
	@echo "  make run INPUT_FILE=photo.bmp OUTPUT_FILE=copy.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=90cw OUTPUT_FILE=rot.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=mirror,90cw OUTPUT_FILE=out.bmp"
	@echo "  make gdb INPUT_FILE=test.bmp ROTATION=180"
	@echo "  make valgrind INPUT_FILE=big.bmp ROTATION=90ccw"
	@echo ""
	@echo "IMPORTANT:"
	@echo "  - INPUT_FILE is required for run/gdb/valgrind targets"
	@echo "  - OUTPUT_FILE defaults to 'output.bmp'"
	@echo "  - ROTATION supports single type or comma-separated batch"
	@echo "  - clean does NOT delete any .bmp files"
	@echo "=================================================================="