/** Build with:
 *   make examples
 */

#include "benchmark.h"
#include <stdlib.h>
#include <string.h>

BENCH_DEFINE(bench_noop, "Empty function call overhead") {
    BENCH_BARRIER();
}

BENCH_DEFINE(bench_add, "Integer addition") {
    volatile int a = 42, b = 58;
    volatile int c = a + b;
    BENCH_KEEP(c);
}

BENCH_DEFINE(bench_malloc_small, "malloc/free 64 bytes") {
    void *p = malloc(64);
    BENCH_KEEP(p);
    free(p);
}

BENCH_DEFINE(bench_malloc_medium, "malloc/free 4KB") {
    void *p = malloc(4096);
    BENCH_KEEP(p);
    free(p);
}

BENCH_DEFINE(bench_malloc_large, "malloc/free 1MB") {
    void *p = malloc(1024 * 1024);
    BENCH_KEEP(p);
    free(p);
}

BENCH_DEFINE(bench_memcpy_small, "memcpy 64 bytes") {
    char src[64] = {0}, dst[64];
    memcpy(dst, src, 64);
    BENCH_KEEP(dst);
}

BENCH_DEFINE(bench_memcpy_medium, "memcpy 4KB") {
    static char src[4096], dst[4096];
    memcpy(dst, src, 4096);
    BENCH_KEEP(dst);
}

BENCH_DEFINE(bench_array_access, "Array random access") {
    static int arr[1024];
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += arr[(i * 7) % 1024];
    }
    BENCH_KEEP(sum);
}

static int __attribute__((noinline)) add_numbers(int a, int b) {
    return a + b;
}

BENCH_DEFINE(bench_function_call, "Function call overhead") {
    volatile int result = add_numbers(42, 58);
    BENCH_KEEP(result);
}

BENCH_DEFINE(bench_division, "Integer division") {
    volatile int a = 1000000, b = 7;
    volatile int c = a / b;
    BENCH_KEEP(c);
}

void bench_custom(void) {
    volatile double x = 3.14159;
    volatile double y = x * x;
    BENCH_KEEP(y);
}

int main(void) {
    bench_register(bench_custom, "bench_custom", "Double multiplication");

    bench_config_t config = {
        .iterations = 10000,
        .warmup_iterations = 1000,
        .output_file = "benchmark_results.csv",
        .verbose = 1
    };
    bench_init_config(&config);

    bench_run_all();
    bench_write_json("benchmark_results.json");
    bench_cleanup();

    return 0;
}

