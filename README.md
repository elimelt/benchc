# benchc

Minimal C benchmarking library with automatic notebook generation.

## Install

```bash
./install.sh
# Add to ~/.bashrc or ~/.zshrc:
export PATH="$PATH:$HOME/.local/bin"
```

## Quick Start

```c
// bench.c
#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"

BENCH(my_function) {
    int result = do_work();
    KEEP(result);
}

int main(void) { return bench_main(); }
```

```bash
benchc bench.c
benchc bench.c -n          # + generate notebook
benchc bench.c -i 10000    # more iterations
benchc bench.c -o results  # output to dir
```

## Macros

### `BENCH(name)`

Define a benchmark function. The name becomes both the function identifier and the display name (stringified).

```c
BENCH(fibonacci_20) {
    int result = fib(20);
    KEEP(result);
}
```

### `KEEP(x)`

Prevents the compiler from optimizing away a computed value. Use on any result you want to force the compiler to actually compute.

```c
BENCH(qsort_1000) {
    int arr[1000];
    for (int i = 0; i < 1000; i++) arr[i] = rand();
    qsort(arr, 1000, sizeof(int), cmp);
    KEEP(arr);  // without this, compiler might skip the whole thing
}
```

### `CLOBBER()`

Memory barrier that forces all pending memory writes. Use when benchmarking code with side effects on memory.

```c
BENCH(memset_4kb) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    CLOBBER();  // force the write to actually happen
}
```

### Combined Example

```c
BENCH(hash_insert_1000) {
    hashtable_t *ht = ht_create();
    for (int i = 0; i < 1000; i++) {
        ht_insert(ht, i, i * 2);
    }
    KEEP(ht);     // don't optimize away the table
    CLOBBER();    // force all writes
    ht_free(ht);
}
```

## Output

Each run produces `benchmark_results.csv`:

```
name,description,iterations,min_ns,max_ns,mean_ns,median_ns,stddev_ns,p95_ns,p99_ns
my_function,my_function,10000,45,892,52.3,48,12.1,67,84
```

With `-n`, also generates `benchmark_analysis.ipynb`. Add `--venv` to create a virtualenv with deps.

## Environment Variables

```bash
BENCH_ITERS=50000 ./mybench    # iterations
BENCH_WARMUP=5000 ./mybench    # warmup iterations
BENCH_QUIET=1 ./mybench        # suppress output
BENCH_CSV=out.csv ./mybench    # output file
```

## Examples

```bash
benchc examples/algorithms/sorting_bench.c -i 500 -n
benchc examples/algorithms/search_bench.c -i 1000
benchc examples/algorithms/hashtable_bench.c -i 1000
benchc examples/algorithms/datastructures_bench.c -i 1000
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
