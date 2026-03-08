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

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/bmp.c $(SRC_DIR)/rotation.c
OBJS = $(SRCS:.c=.o)
HEADERS = $(INC_DIR)/bmp.h $(INC_DIR)/rotation.h 

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
	@echo "Usage: make <target> INPUT_FILE=bmp_img/photo.bmp [OUTPUT_FILE=bmp_img/out.bmp] [ROTATION=type]"
	@echo ""
	@echo "Examples:"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp"
	@echo "  make run INPUT_FILE=bmp_img/mr.bean.bmp OUTPUT_FILE=bmp_img/copy.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp ROTATION=90cw OUTPUT_FILE=bmp_img/rotated.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp ROTATION=hflip,90cw OUTPUT_FILE=bmp_img/complex.bmp"
	@echo "  make gdb INPUT_FILE=bmp_img/test.bmp ROTATION=180"
	@echo "  make valgrind INPUT_FILE=bmp_img/big.bmp ROTATION=90ccw OUTPUT_FILE=bmp_img/result.bmp"
	@echo ""
	@exit 1
endif

# If INPUT_FILE doesn't contain a directory, assume it's in IMG_DIR
INPUT_PATH := $(if $(dir $(INPUT_FILE)),$(INPUT_FILE),$(IMG_DIR)/$(INPUT_FILE))

# If OUTPUT_FILE doesn't contain a directory, put it in IMG_DIR
OUTPUT_PATH := $(if $(dir $(OUTPUT_FILE)),$(OUTPUT_FILE),$(IMG_DIR)/$(OUTPUT_FILE))

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
	@echo ""
	@echo "RUN TARGETS (INPUT_FILE is REQUIRED):"
	@echo "  make run INPUT_FILE=photo.bmp                   # Assumes bmp_img/photo.bmp"
	@echo "  make run INPUT_FILE=bmp_img/photo.bmp           # Explicit path"
	@echo "  make run INPUT_FILE=a.bmp ROTATION=90cw         # Rotate 90° CW"
	@echo "  make run INPUT_FILE=a.bmp ROTATION=90cw,hflip   # Batch rotation"
	@echo "  make gdb INPUT_FILE=your.bmp [ROTATION=type]    # Debug with GDB"
	@echo "  make valgrind INPUT_FILE=your.bmp [ROTATION=t]  # Check memory"
	@echo ""
	@echo "VARIABLES:"
	@echo "  INPUT_FILE   : Source BMP file (basename assumes bmp_img/, or use full path)"
	@echo "  OUTPUT_FILE  : Destination file (default: bmp_img/output.bmp)"
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
	@echo "  - If no path given, assumes file is in bmp_img/"
	@echo "  - OUTPUT_FILE defaults to 'bmp_img/output.bmp'"
	@echo "  - ROTATION supports single type or comma-separated batch"
	@echo "  - clean does NOT delete any .bmp files"
	@echo "=================================================================="