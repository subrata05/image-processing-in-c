# =============================================================================
# BMP Loader Makefile
# =============================================================================

# =============================================================================
# COMPILER SETTINGS
# =============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99
TARGET = img-bin
SRCS = main.c bmp.c

# =============================================================================
# USER CONFIGURABLE VARIABLES
# =============================================================================

# INPUT_FILE: REQUIRED - must be specified by user
# No default here! User MUST provide: make run INPUT_FILE=photo.bmp
INPUT_FILE ?=

# OUTPUT_FILE: Optional - defaults to output.bmp if not specified
OUTPUT_FILE ?= output.bmp

# =============================================================================
# BUILD TARGETS
# =============================================================================

.PHONY: all clean debug release gdb valgrind run help check-input

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
$(TARGET): $(SRCS) bmp.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# =============================================================================
# INPUT VALIDATION
# =============================================================================

# This target checks if INPUT_FILE is provided
check-input:
ifeq ($(INPUT_FILE),)
	@echo "ERROR: No input file specified!"
	@echo ""
	@echo "Usage: make <target> INPUT_FILE=your.bmp [OUTPUT_FILE=out.bmp]"
	@echo ""
	@echo "Examples:"
	@echo "  make run INPUT_FILE=photo.bmp"
	@echo "  make run INPUT_FILE=mr.bean.bmp OUTPUT_FILE=copy.bmp"
	@echo "  make gdb INPUT_FILE=test.bmp"
	@echo "  make valgrind INPUT_FILE=big.bmp OUTPUT_FILE=result.bmp"
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
	@echo "Starting GDB with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)"
	gdb -x .gdbinit --args ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)

# -----------------------------------------------------------------------------
# TARGET: valgrind (memory check)
# -----------------------------------------------------------------------------
valgrind: debug check-input
	@echo "Running Valgrind with: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)

# -----------------------------------------------------------------------------
# TARGET: run (quick test)
# -----------------------------------------------------------------------------
run: debug check-input
	@echo "Running: ./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)"
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)

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
	@echo "BMP Loader Makefile"
	@echo "=================================================================="
	@echo ""
	@echo "BUILD TARGETS (no input file needed):"
	@echo "  make              Build release version (optimized)"
	@echo "  make release      Same as 'make'"
	@echo "  make debug        Build debug version (for GDB/Valgrind)"
	@echo "  make clean        Remove executable and GDB history"
	@echo ""
	@echo "RUN TARGETS (INPUT_FILE is REQUIRED):"
	@echo "  make run INPUT_FILE=your.bmp              # Output: output.bmp"
	@echo "  make run INPUT_FILE=a.bmp OUTPUT_FILE=b.bmp  # Output: b.bmp"
	@echo "  make gdb INPUT_FILE=your.bmp              # Debug with GDB"
	@echo "  make valgrind INPUT_FILE=your.bmp         # Check memory leaks"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make run INPUT_FILE=mr.bean.bmp"
	@echo "  make run INPUT_FILE=photo.bmp OUTPUT_FILE=copy.bmp"
	@echo "  make gdb INPUT_FILE=test.bmp"
	@echo "  make valgrind INPUT_FILE=big.bmp OUTPUT_FILE=result.bmp"
	@echo ""
	@echo "IMPORTANT:"
	@echo "  - INPUT_FILE is required for run/gdb/valgrind targets"
	@echo "  - OUTPUT_FILE defaults to 'output.bmp' if not specified"
	@echo "  - clean does NOT delete any .bmp files (input or output)"
	@echo "=================================================================="