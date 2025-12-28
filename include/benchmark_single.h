#ifndef BENCHMARK_SINGLE_H
#define BENCHMARK_SINGLE_H

#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BENCH_ITERATIONS
#define BENCH_ITERATIONS 1000
#endif
#ifndef BENCH_WARMUP
#define BENCH_WARMUP 100
#endif
#ifndef BENCH_MAX_BENCHMARKS
#define BENCH_MAX_BENCHMARKS 256
#endif
#ifndef BENCH_MAX_NAME
#define BENCH_MAX_NAME 128
#endif

typedef void (*bench_fn_t)(void);
typedef struct { double min_ns, max_ns, mean_ns, median_ns, stddev_ns, p95_ns, p99_ns; uint64_t iterations; } bench_stats_t;
typedef struct { char name[BENCH_MAX_NAME]; char desc[BENCH_MAX_NAME]; bench_fn_t fn; bench_stats_t stats; } bench_entry_t;

void bench_register(bench_fn_t fn, const char *name, const char *desc);
int bench_main(void);
uint64_t bench_now(void);
void bench_escape(void *p);
void bench_clobber(void);

#define BENCH(name, desc) \
    static void name(void); \
    __attribute__((constructor)) static void _reg_##name(void) { bench_register(name, #name, desc); } \
    static void name(void)

#define KEEP(x) bench_escape((void*)&(x))
#define CLOBBER() bench_clobber()

#ifdef __cplusplus
}
#endif

#ifdef BENCHMARK_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static struct {
    bench_entry_t entries[BENCH_MAX_BENCHMARKS];
    size_t count; int quiet; uint64_t iters, warmup;
    const char *csv_file;
} _bench = {.iters = BENCH_ITERATIONS, .warmup = BENCH_WARMUP, .csv_file = "benchmark_results.csv"};

uint64_t bench_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void bench_escape(void *p) { __asm__ volatile("" : : "r,m"(p) : "memory"); }
void bench_clobber(void) { __asm__ volatile("" : : : "memory"); }

void bench_register(bench_fn_t fn, const char *name, const char *desc) {
    if (_bench.count >= BENCH_MAX_BENCHMARKS) return;
    bench_entry_t *e = &_bench.entries[_bench.count++];
    e->fn = fn;
    strncpy(e->name, name, BENCH_MAX_NAME - 1);
    strncpy(e->desc, desc ? desc : "", BENCH_MAX_NAME - 1);
}

static int _cmp_dbl(const void *a, const void *b) {
    double d = *(double*)a - *(double*)b;
    return (d > 0) - (d < 0);
}

static void _run_one(bench_entry_t *e) {
    for (uint64_t i = 0; i < _bench.warmup; i++) e->fn();
    double *samples = malloc(_bench.iters * sizeof(double));
    for (uint64_t i = 0; i < _bench.iters; i++) {
        uint64_t t0 = bench_now();
        e->fn();
        samples[i] = (double)(bench_now() - t0);
    }
    qsort(samples, _bench.iters, sizeof(double), _cmp_dbl);
    bench_stats_t *s = &e->stats;
    s->iterations = _bench.iters;
    s->min_ns = samples[0];
    s->max_ns = samples[_bench.iters - 1];
    s->median_ns = samples[_bench.iters / 2];
    s->p95_ns = samples[(size_t)(_bench.iters * 0.95)];
    s->p99_ns = samples[(size_t)(_bench.iters * 0.99)];
    double sum = 0;
    for (size_t i = 0; i < _bench.iters; i++) sum += samples[i];
    s->mean_ns = sum / _bench.iters;
    double var = 0;
    for (size_t i = 0; i < _bench.iters; i++) { double d = samples[i] - s->mean_ns; var += d * d; }
    s->stddev_ns = sqrt(var / _bench.iters);
    free(samples);
}

static void _write_csv(void) {
    FILE *f = fopen(_bench.csv_file, "w");
    if (!f) return;
    fprintf(f, "name,description,iterations,min_ns,max_ns,mean_ns,median_ns,stddev_ns,p95_ns,p99_ns\n");
    for (size_t i = 0; i < _bench.count; i++) {
        bench_entry_t *e = &_bench.entries[i];
        fprintf(f, "%s,\"%s\",%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            e->name, e->desc, (unsigned long)e->stats.iterations,
            e->stats.min_ns, e->stats.max_ns, e->stats.mean_ns, e->stats.median_ns,
            e->stats.stddev_ns, e->stats.p95_ns, e->stats.p99_ns);
    }
    fclose(f);
}

static void _print_results(void) {
    printf("\n%-30s %10s %10s %10s %10s\n", "Benchmark", "Mean(ns)", "Median", "StdDev", "P99");
    printf("%-30s %10s %10s %10s %10s\n", "─────────", "────────", "──────", "──────", "───");
    for (size_t i = 0; i < _bench.count; i++) {
        bench_entry_t *e = &_bench.entries[i];
        printf("%-30s %10.1f %10.1f %10.1f %10.1f\n",
            e->name, e->stats.mean_ns, e->stats.median_ns, e->stats.stddev_ns, e->stats.p99_ns);
    }
    printf("\n");
}

int bench_main(void) {
    char *env;
    if ((env = getenv("BENCH_ITERS"))) _bench.iters = (uint64_t)atol(env);
    if ((env = getenv("BENCH_WARMUP"))) _bench.warmup = (uint64_t)atol(env);
    if ((env = getenv("BENCH_CSV"))) _bench.csv_file = env;
    if ((env = getenv("BENCH_QUIET"))) _bench.quiet = atoi(env);
    if (!_bench.quiet)
        printf("Running %zu benchmarks (%lu iterations, %lu warmup)...\n",
            _bench.count, (unsigned long)_bench.iters, (unsigned long)_bench.warmup);
    for (size_t i = 0; i < _bench.count; i++) {
        if (!_bench.quiet) printf("  %s\r", _bench.entries[i].name);
        fflush(stdout);
        _run_one(&_bench.entries[i]);
    }
    if (!_bench.quiet) _print_results();
    _write_csv();
    if (!_bench.quiet) printf("Results: %s\n", _bench.csv_file);
    return 0;
}

#endif
#endif
