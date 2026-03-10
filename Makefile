# =============================================================================
# BMP Image Processor Makefile
# =============================================================================

# =============================================================================
# COMPILER SETTINGS
# =============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99
TARGET = img-processor

# Source paths
SRC_DIR = src
INC_DIR = include
IMG_DIR = bmp_img

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/bmp.c $(SRC_DIR)/rotation.c $(SRC_DIR)/filter.c
OBJS = $(SRCS:.c=.o)
HEADERS = $(INC_DIR)/bmp.h $(INC_DIR)/rotation.h $(INC_DIR)/filter.h

# Include path for compiler
CFLAGS += -I$(INC_DIR)

# =============================================================================
# USER CONFIGURABLE VARIABLES
# =============================================================================

# INPUT_FILE: REQUIRED - must be specified by user
# No default here! User MUST provide: make run INPUT_FILE=photo.bmp
INPUT_FILE ?=

# OUTPUT_FILE: Optional - defaults to output.bmp if not specified
OUTPUT_FILE ?= $(IMG_DIR)/output.bmp

# ROTATION: Optional - rotation type (90cw, 90ccw, 180, hflip, vflip, etc.)
# Can specify multiple rotations separated by commas for batch processing
# Examples: ROTATION=90cw | ROTATION=hflip,90cw | ROTATION=180
ROTATION ?=

# FILTER: Optional - filter type (grayscale, negative)
# Examples: FILTER=grayscale | FILTER=negative
FILTER ?=

# =============================================================================
# BUILD TARGETS
# =============================================================================

.PHONY: all clean debug release gdb valgrind run help check-input list-rotations list-filters

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
$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

# =============================================================================
# INPUT VALIDATION
# =============================================================================

# This target checks if INPUT_FILE is provided
check-input:
ifeq ($(INPUT_FILE),)
	@echo "ERROR: No input file specified!"
	@echo ""
	@echo "Usage: make <target> INPUT_FILE=bmp_img/photo.bmp [OUTPUT_FILE=bmp_img/out.bmp] [ROTATION=type] [FILTER=type]"
	@echo ""
	@echo "Examples:"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp FILTER=grayscale OUTPUT_FILE=bmp_img/gray.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp FILTER=negative OUTPUT_FILE=bmp_img/neg.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw OUTPUT_FILE=bmp_img/rotated.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp ROTATION=hflip,90cw OUTPUT_FILE=bmp_img/complex.bmp"
	@echo "  make gdb INPUT_FILE=bmp_img/test.bmp FILTER=grayscale"
	@echo "  make valgrind INPUT_FILE=bmp_img/big.bmp ROTATION=90ccw OUTPUT_FILE=bmp_img/result.bmp"
	@echo ""
	@exit 1
endif

# If INPUT_FILE doesn't contain a directory, assume it's in IMG_DIR
INPUT_PATH := $(if $(dir $(INPUT_FILE)),$(INPUT_FILE),$(IMG_DIR)/$(INPUT_FILE))

# If OUTPUT_FILE doesn't contain a directory, put it in IMG_DIR
OUTPUT_PATH := $(if $(dir $(OUTPUT_FILE)),$(OUTPUT_FILE),$(IMG_DIR)/$(OUTPUT_FILE))

# =============================================================================
# BUILD OPERATION FLAGS
# =============================================================================

# Rotation flags (--rotate / --batch)
ROTATION_FLAGS := $(if $(ROTATION),$(if $(findstring ,,$(ROTATION)),--batch $(ROTATION),--rotate $(ROTATION)),)

# Filter flag (--filter)
FILTER_FLAGS := $(if $(FILTER),--filter $(FILTER),)

# Combined operation flags
OP_FLAGS := $(ROTATION_FLAGS) $(FILTER_FLAGS)

# =============================================================================
# UTILITY TARGETS (all require input file)
# =============================================================================

# -----------------------------------------------------------------------------
# TARGET: gdb (start debugger)
# -----------------------------------------------------------------------------
gdb: debug check-input
	@echo "Starting GDB with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)"
	gdb -x .gdbinit --args ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)

# -----------------------------------------------------------------------------
# TARGET: valgrind (memory check)
# -----------------------------------------------------------------------------
valgrind: debug check-input
	@echo "Running Valgrind with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)

# -----------------------------------------------------------------------------
# TARGET: run (quick test)
# -----------------------------------------------------------------------------
run: debug check-input
	@echo "Running: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)"
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE) $(OP_FLAGS)

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

# -----------------------------------------------------------------------------
# TARGET: list-filters (show available filter types)
# -----------------------------------------------------------------------------
list-filters: debug
	@echo "Available filter types:"
	@echo "  gray, grey, grayscale, gs   : Convert to grayscale"
	@echo "  neg, negative, invert, inv  : Negative (invert colors)"

# =============================================================================
# UTILITY TARGETS (no input required)
# =============================================================================

# -----------------------------------------------------------------------------
# TARGET: clean
# -----------------------------------------------------------------------------
clean:
	rm -f $(TARGET) $(SRC_DIR)/*.o .gdb_history
	@echo "Cleaned: executable, object files, and GDB history"
	@echo "Note: Image files in $(IMG_DIR)/ are NOT deleted"

# -----------------------------------------------------------------------------
# TARGET: help
# -----------------------------------------------------------------------------
help:
	@echo "=================================================================="
	@echo "BMP Image Processor Makefile"
	@echo "=================================================================="
	@echo ""
	@echo "FOLDER STRUCTURE:"
	@echo "  include/          Header files (.h)"
	@echo "  src/              Source files (.c)"
	@echo "  bmp_img/          Image files (.bmp)"
	@echo ""
	@echo "BUILD TARGETS (no input file needed):"
	@echo "  make              Build release version (optimized)"
	@echo "  make release      Same as 'make'"
	@echo "  make debug        Build debug version (for GDB/Valgrind)"
	@echo "  make clean        Remove executable and GDB history"
	@echo "  make list-rotations   Show available rotation types"
	@echo "  make list-filters     Show available filter types"
	@echo ""
	@echo "RUN TARGETS (INPUT_FILE is REQUIRED):"
	@echo "  make run INPUT_FILE=photo.bmp                        # Copy only"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=90cw          # Rotate 90 CW"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=90cw,hflip    # Batch rotation"quick
	@echo "  make run INPUT_FILE=photo.bmp FILTER=grayscale       # Grayscale"
	@echo "  make run INPUT_FILE=photo.bmp FILTER=negative        # Negative"
	@echo "  make gdb INPUT_FILE=your.bmp [ROTATION=type]         # Debug with GDB"
	@echo "  make valgrind INPUT_FILE=your.bmp [FILTER=type]      # Check memory"
	@echo ""
	@echo "VARIABLES:"
	@echo "  INPUT_FILE   : Source BMP file (basename assumes bmp_img/, or full path)"
	@echo "  OUTPUT_FILE  : Destination file (default: bmp_img/output.bmp)"
	@echo "  ROTATION     : Rotation type or batch (comma-separated)"
	@echo "  FILTER       : Filter type (grayscale, negative)"
	@echo ""
	@echo "DIRECT BINARY USAGE (mix rotations and filters):"
	@echo "  ./img-processor photo.bmp --rotate 90cw --filter gray -o out.bmp"
	@echo "  ./img-processor photo.bmp --filter negative --rotate hflip -o out.bmp"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make run INPUT_FILE=mr.bean.bmp FILTER=grayscale"
	@echo "  make run INPUT_FILE=photo.bmp FILTER=negative OUTPUT_FILE=neg.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=90cw OUTPUT_FILE=rot.bmp"
	@echo "  make run INPUT_FILE=photo.bmp ROTATION=mirror,90cw OUTPUT_FILE=out.bmp"
	@echo "  make gdb INPUT_FILE=test.bmp FILTER=grayscale"
	@echo "  make valgrind INPUT_FILE=big.bmp ROTATION=90ccw"
	@echo ""
	@echo "IMPORTANT:"
	@echo "  - INPUT_FILE is required for run/gdb/valgrind targets"
	@echo "  - If no path given, assumes file is in bmp_img/"
	@echo "  - OUTPUT_FILE defaults to 'bmp_img/output.bmp'"
	@echo "  - ROTATION and FILTER can both be used in the same make run command"
	@echo "    but only one of each per make invocation (use binary directly for"
	@echo "    complex mixed pipelines)"
	@echo "  - clean does NOT delete any .bmp files"
	@echo "=================================================================="