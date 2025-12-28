# benchc - C Benchmarking Library
# Makefile

CC ?= gcc
AR ?= ar
CFLAGS ?= -O2 -Wall -Wextra -pedantic -std=c11
LDFLAGS ?= -lm

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
LIB_DIR = build/lib
EXAMPLES_DIR = examples

# Sources
LIB_SRCS = $(SRC_DIR)/benchmark.c
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# Library
LIB_NAME = libbenchmark
STATIC_LIB = $(LIB_DIR)/$(LIB_NAME).a

# Example
EXAMPLE_SRC = $(EXAMPLES_DIR)/example_bench.c
EXAMPLE_BIN = $(BUILD_DIR)/example_bench

# Targets
.PHONY: all lib examples clean run-example notebook help

all: lib examples

help:
	@echo "benchc - C Benchmarking Library"
	@echo ""
	@echo "Targets:"
	@echo "  all          Build library and examples"
	@echo "  lib          Build static library"
	@echo "  examples     Build example benchmarks"
	@echo "  run-example  Run example and generate notebook"
	@echo "  notebook     Generate analysis notebook from CSV"
	@echo "  clean        Remove build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  CC           C compiler (default: gcc)"
	@echo "  CFLAGS       Compiler flags"

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Compile library objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Build static library
lib: $(STATIC_LIB)

$(STATIC_LIB): $(LIB_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "Built: $@"

# Build examples
examples: $(EXAMPLE_BIN)

$(EXAMPLE_BIN): $(EXAMPLE_SRC) $(STATIC_LIB)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< -L$(LIB_DIR) -lbenchmark $(LDFLAGS) -o $@
	@echo "Built: $@"

# Run example and generate notebook
run-example: $(EXAMPLE_BIN)
	@echo "Running example benchmark..."
	cd $(BUILD_DIR) && ./example_bench
	@echo ""
	@echo "Generating analysis notebook..."
	python3 scripts/generate_notebook.py $(BUILD_DIR)/benchmark_results.csv -o $(BUILD_DIR)/benchmark_analysis.ipynb
	@echo ""
	@echo "Done! Open $(BUILD_DIR)/benchmark_analysis.ipynb to analyze results."

# Generate notebook from existing CSV
notebook:
	@if [ -z "$(CSV)" ]; then \
		echo "Usage: make notebook CSV=path/to/results.csv [OUTPUT=notebook.ipynb]"; \
		exit 1; \
	fi
	python3 scripts/generate_notebook.py $(CSV) -o $(or $(OUTPUT),benchmark_analysis.ipynb)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)
	rm -f benchmark_results.csv benchmark_results.json benchmark_analysis.ipynb

# Install (optional)
PREFIX ?= /usr/local

install: lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(INC_DIR)/benchmark.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(STATIC_LIB) $(DESTDIR)$(PREFIX)/lib/

