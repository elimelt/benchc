# benchc

Minimal C benchmarking library with automatic notebook generation.

## Quick Start

```c
// bench.c
#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"

BENCH(my_function, "description") {
    int result = do_work();
    KEEP(result);
}

int main(void) { return bench_main(); }
```

```bash
./scripts/bench.sh bench.c
./scripts/bench.sh bench.c -n          # + generate notebook
./scripts/bench.sh bench.c -i 10000    # more iterations
./scripts/bench.sh bench.c -o results  # output to dir
```

## Macros

| Macro | Description |
|-------|-------------|
| `BENCH(name, desc)` | Define a benchmark |
| `KEEP(x)` | Prevent compiler from optimizing away `x` |
| `CLOBBER()` | Memory barrier, force memory writes |

## Output

Each run produces `benchmark_results.csv`:

```
name,description,iterations,min_ns,max_ns,mean_ns,median_ns,stddev_ns,p95_ns,p99_ns
my_function,description,10000,45,892,52.3,48,12.1,67,84
```

With `-n`, also generates `benchmark_analysis.ipynb` with pre-loaded charts and a `.venv` with deps.

## Environment Variables

```bash
BENCH_ITERS=50000 ./mybench    # iterations
BENCH_WARMUP=5000 ./mybench    # warmup iterations
BENCH_QUIET=1 ./mybench        # suppress output
BENCH_CSV=out.csv ./mybench    # output file
```

## Examples

```bash
./scripts/bench.sh examples/algorithms/sorting_bench.c -i 500 -n
./scripts/bench.sh examples/algorithms/search_bench.c -i 1000
./scripts/bench.sh examples/algorithms/hashtable_bench.c -i 1000
./scripts/bench.sh examples/algorithms/datastructures_bench.c -i 1000
```

## Library Mode

For larger projects, use the separate library:

```bash
make lib  # builds build/lib/libbenchmark.a
```

```c
// mybench.c
#include "benchmark.h"

BENCH_DEFINE(my_function, "description") {
    int result = do_work();
    BENCH_KEEP(result);
}

int main(void) {
    bench_init();
    bench_run_all();
    return 0;
}
```

```bash
gcc -I include mybench.c -L build/lib -lbenchmark -o mybench
./mybench
```

## API

```c
void bench_init(void);
void bench_init_config(bench_config_t *cfg);
void bench_run_all(void);
void bench_register(bench_fn fn, const char *name, const char *desc);
void bench_write_json(const char *path);
void bench_cleanup(void);
```

## License

Public domain.
