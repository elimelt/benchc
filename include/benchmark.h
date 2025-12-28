#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BENCH_MAX_BENCHMARKS 256
#define BENCH_MAX_NAME_LEN 128
#define BENCH_DEFAULT_ITERATIONS 1000
#define BENCH_DEFAULT_WARMUP 100

typedef void (*bench_fn_t)(void);

typedef struct {
    double min_ns, max_ns, mean_ns, median_ns, stddev_ns, p95_ns, p99_ns;
    uint64_t iterations;
} bench_stats_t;

typedef struct {
    char name[BENCH_MAX_NAME_LEN];
    char description[BENCH_MAX_NAME_LEN];
    bench_stats_t stats;
} bench_result_t;

typedef struct {
    uint64_t iterations;
    uint64_t warmup_iterations;
    const char *output_file;
    int verbose;
} bench_config_t;

void bench_init(void);
void bench_init_config(const bench_config_t *config);
void bench_register(bench_fn_t fn, const char *name, const char *description);
int bench_run_all(void);
int bench_run(const char *name);
const bench_result_t *bench_get_results(size_t *count);
int bench_write_csv(const char *filename);
int bench_write_json(const char *filename);
void bench_cleanup(void);
uint64_t bench_timestamp_ns(void);
void bench_do_not_optimize(void *ptr);
void bench_clobber(void);

#define BENCH_REGISTER(fn, desc) \
    __attribute__((constructor)) static void _bench_register_##fn(void) { \
        bench_register(fn, #fn, desc); \
    }

#define BENCH_DEFINE(name, desc) \
    static void name(void); \
    BENCH_REGISTER(name, desc) \
    static void name(void)

#define BENCH_KEEP(x) bench_do_not_optimize((void*)&(x))
#define BENCH_BARRIER() bench_clobber()

#ifdef __cplusplus
}
#endif

#endif

