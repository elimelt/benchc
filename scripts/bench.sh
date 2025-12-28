#!/bin/bash
# bench.sh - Quick compile-and-run for benchmark files
#
# Usage: ./scripts/bench.sh mytest.c [options]
#
# Fast iteration loop for C benchmarking:
#   1. Compiles your benchmark file (single-header or linked)
#   2. Runs benchmarks with configurable iterations
#   3. Outputs CSV (and optionally generates Jupyter notebook)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'

usage() {
    cat << EOF
Usage: $0 <source.c> [options]

Options:
  -n, --notebook     Generate Jupyter notebook
  -o, --output DIR   Output directory (default: .)
  -i, --iters N      Iteration count (default: 1000)
  -w, --warmup N     Warmup iterations (default: 100)
  -q, --quiet        Minimal output
  -s, --single       Use single-header mode (no library linking)
  --venv             Create venv with notebook deps
  -h, --help         Show this help

Environment variables:
  BENCH_ITERS        Override iteration count
  BENCH_WARMUP       Override warmup count
  BENCH_CSV          Override output CSV path
  BENCH_QUIET        Set to 1 for quiet mode

Examples:
  $0 mybench.c                    # Quick run
  $0 mybench.c -n                 # Run + notebook
  $0 mybench.c -i 10000 -n        # More iterations + notebook
  BENCH_ITERS=50000 $0 mybench.c  # Via env var
EOF
    exit 1
}

# Defaults
NOTEBOOK=0; OUTPUT_DIR="."; QUIET=0; SINGLE=0; VENV=0; ITERS=""; WARMUP=""; SOURCE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -n|--notebook) NOTEBOOK=1; shift ;;
        -o|--output) OUTPUT_DIR="$2"; shift 2 ;;
        -i|--iters) ITERS="$2"; shift 2 ;;
        -w|--warmup) WARMUP="$2"; shift 2 ;;
        -q|--quiet) QUIET=1; shift ;;
        -s|--single) SINGLE=1; shift ;;
        --venv) VENV=1; shift ;;
        -h|--help) usage ;;
        -*) echo -e "${RED}Unknown option: $1${NC}"; usage ;;
        *) SOURCE="$1"; shift ;;
    esac
done

[[ -z "$SOURCE" ]] && { echo -e "${RED}Error: No source file${NC}"; usage; }
[[ ! -f "$SOURCE" ]] && { echo -e "${RED}Error: File not found: $SOURCE${NC}"; exit 1; }

BASENAME=$(basename "$SOURCE" .c)
BINARY="${OUTPUT_DIR}/${BASENAME}"
CSV="${OUTPUT_DIR}/benchmark_results.csv"

mkdir -p "$OUTPUT_DIR"

# Set env vars for runtime config
[[ -n "$ITERS" ]] && export BENCH_ITERS="$ITERS"
[[ -n "$WARMUP" ]] && export BENCH_WARMUP="$WARMUP"
[[ $QUIET -eq 1 ]] && export BENCH_QUIET=1

# Compile
if [[ $SINGLE -eq 1 ]] || grep -q "BENCHMARK_IMPLEMENTATION" "$SOURCE" 2>/dev/null; then
    # Single-header mode - no library needed
    [[ $QUIET -eq 0 ]] && echo -e "${GREEN}Compiling (single-header)...${NC}"
    gcc -O2 -I"${ROOT_DIR}/include" "$SOURCE" -lm -o "$BINARY"
else
    # Library mode - build lib if needed
    LIB="${ROOT_DIR}/build/lib/libbenchmark.a"
    if [[ ! -f "$LIB" ]]; then
        [[ $QUIET -eq 0 ]] && echo -e "${YELLOW}Building library...${NC}"
        make -C "$ROOT_DIR" lib >/dev/null 2>&1
    fi
    [[ $QUIET -eq 0 ]] && echo -e "${GREEN}Compiling...${NC}"
    gcc -O2 -I"${ROOT_DIR}/include" "$SOURCE" -L"${ROOT_DIR}/build/lib" -lbenchmark -lm -o "$BINARY"
fi

# Run
(cd "$OUTPUT_DIR" && "./$BASENAME")

# Notebook
if [[ $NOTEBOOK -eq 1 ]]; then
    NB="${OUTPUT_DIR}/benchmark_analysis.ipynb"
    VENV_FLAG=""
    [[ $VENV -eq 1 ]] && VENV_FLAG="--venv"
    python3 "${ROOT_DIR}/scripts/generate_notebook.py" "$CSV" -o "$NB" $VENV_FLAG
    [[ $QUIET -eq 0 ]] && echo -e "${GREEN}Notebook: ${NB}${NC}"
fi

