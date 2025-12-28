#define _POSIX_C_SOURCE 199309L

#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct
{
    bench_fn_t fn;
    char name[BENCH_MAX_NAME_LEN];
    char description[BENCH_MAX_NAME_LEN];
} bench_entry_t;

static struct
{
    bench_entry_t benchmarks[BENCH_MAX_BENCHMARKS];
    bench_result_t results[BENCH_MAX_BENCHMARKS];
    size_t count;
    size_t result_count;
    bench_config_t config;
    int initialized;
} g_bench = {0};

static int compare_double(const void *a, const void *b)
{
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

static void calculate_stats(double *samples, size_t n, bench_stats_t *stats)
{
    if (n == 0)
        return;
    qsort(samples, n, sizeof(double), compare_double);
    stats->min_ns = samples[0];
    stats->max_ns = samples[n - 1];
    stats->median_ns = samples[n / 2];
    stats->iterations = n;
    double sum = 0.0;
    for (size_t i = 0; i < n; i++)
        sum += samples[i];
    stats->mean_ns = sum / n;
    double sq_diff_sum = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        double diff = samples[i] - stats->mean_ns;
        sq_diff_sum += diff * diff;
    }
    stats->stddev_ns = sqrt(sq_diff_sum / n);
    stats->p95_ns = samples[(size_t)(n * 0.95)];
    stats->p99_ns = samples[(size_t)(n * 0.99)];
}

uint64_t bench_timestamp_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void bench_do_not_optimize(void *ptr)
{
    __asm__ volatile("" : : "r,m"(ptr) : "memory");
}

void bench_clobber(void)
{
    __asm__ volatile("" : : : "memory");
}

void bench_init(void)
{
    bench_config_t config = {
        .iterations = BENCH_DEFAULT_ITERATIONS,
        .warmup_iterations = BENCH_DEFAULT_WARMUP,
        .output_file = "benchmark_results.csv",
        .verbose = 1};
    char *env;
    if ((env = getenv("BENCH_ITERS")))
        config.iterations = (uint64_t)atol(env);
    if ((env = getenv("BENCH_WARMUP")))
        config.warmup_iterations = (uint64_t)atol(env);
    if ((env = getenv("BENCH_CSV")))
        config.output_file = env;
    if ((env = getenv("BENCH_QUIET")))
        config.verbose = !atoi(env);
    bench_init_config(&config);
}

void bench_init_config(const bench_config_t *config)
{
    g_bench.result_count = 0;
    if (config)
        g_bench.config = *config;
    g_bench.initialized = 1;
}

void bench_register(bench_fn_t fn, const char *name, const char *description)
{
    if (g_bench.count >= BENCH_MAX_BENCHMARKS)
        return;
    bench_entry_t *entry = &g_bench.benchmarks[g_bench.count++];
    entry->fn = fn;
    strncpy(entry->name, name, BENCH_MAX_NAME_LEN - 1);
    strncpy(entry->description, description ? description : "", BENCH_MAX_NAME_LEN - 1);
}

static int run_single_benchmark(bench_entry_t *entry, bench_result_t *result)
{
    const uint64_t warmup = g_bench.config.warmup_iterations;
    const uint64_t iters = g_bench.config.iterations;

    if (g_bench.config.verbose)
    {
        printf("Running: %s (%s)\n", entry->name, entry->description);
        printf("  Warmup: %lu iterations\n", (unsigned long)warmup);
    }

    for (uint64_t i = 0; i < warmup; i++)
        entry->fn();

    double *samples = malloc(iters * sizeof(double));
    if (!samples)
        return -1;

    if (g_bench.config.verbose)
        printf("  Timing: %lu iterations\n", (unsigned long)iters);

    for (uint64_t i = 0; i < iters; i++)
    {
        uint64_t start = bench_timestamp_ns();
        entry->fn();
        samples[i] = (double)(bench_timestamp_ns() - start);
    }

    strncpy(result->name, entry->name, BENCH_MAX_NAME_LEN - 1);
    strncpy(result->description, entry->description, BENCH_MAX_NAME_LEN - 1);
    calculate_stats(samples, iters, &result->stats);
    free(samples);

    if (g_bench.config.verbose)
    {
        printf("  Mean: %.2f ns, Median: %.2f ns, StdDev: %.2f ns\n",
               result->stats.mean_ns, result->stats.median_ns, result->stats.stddev_ns);
        printf("  Min: %.2f ns, Max: %.2f ns\n", result->stats.min_ns, result->stats.max_ns);
        printf("  P95: %.2f ns, P99: %.2f ns\n\n", result->stats.p95_ns, result->stats.p99_ns);
    }

    return 0;
}

int bench_run_all(void)
{
    if (!g_bench.initialized)
        bench_init();
    if (g_bench.config.verbose)
        printf("=== Running %zu benchmarks ===\n\n", g_bench.count);
    g_bench.result_count = 0;
    for (size_t i = 0; i < g_bench.count; i++)
    {
        if (run_single_benchmark(&g_bench.benchmarks[i], &g_bench.results[g_bench.result_count]) == 0)
            g_bench.result_count++;
    }
    if (g_bench.config.output_file)
        bench_write_csv(g_bench.config.output_file);
    return 0;
}

int bench_run(const char *name)
{
    if (!g_bench.initialized)
        bench_init();
    for (size_t i = 0; i < g_bench.count; i++)
    {
        if (strcmp(g_bench.benchmarks[i].name, name) == 0)
            return run_single_benchmark(&g_bench.benchmarks[i], &g_bench.results[g_bench.result_count++]);
    }
    return -1;
}

const bench_result_t *bench_get_results(size_t *count)
{
    if (count)
        *count = g_bench.result_count;
    return g_bench.results;
}

int bench_write_csv(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
        return -1;
    fprintf(fp, "name,description,iterations,min_ns,max_ns,mean_ns,median_ns,stddev_ns,p95_ns,p99_ns\n");
    for (size_t i = 0; i < g_bench.result_count; i++)
    {
        bench_result_t *r = &g_bench.results[i];
        fprintf(fp, "%s,\"%s\",%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                r->name, r->description, (unsigned long)r->stats.iterations,
                r->stats.min_ns, r->stats.max_ns, r->stats.mean_ns, r->stats.median_ns,
                r->stats.stddev_ns, r->stats.p95_ns, r->stats.p99_ns);
    }
    fclose(fp);
    if (g_bench.config.verbose)
        printf("Results written to %s\n", filename);
    return 0;
}

int bench_write_json(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
        return -1;
    fprintf(fp, "{\n  \"benchmarks\": [\n");
    for (size_t i = 0; i < g_bench.result_count; i++)
    {
        bench_result_t *r = &g_bench.results[i];
        fprintf(fp, "    {\"name\":\"%s\",\"description\":\"%s\",\"iterations\":%lu,"
                    "\"min_ns\":%.2f,\"max_ns\":%.2f,\"mean_ns\":%.2f,\"median_ns\":%.2f,"
                    "\"stddev_ns\":%.2f,\"p95_ns\":%.2f,\"p99_ns\":%.2f}%s\n",
                r->name, r->description, (unsigned long)r->stats.iterations,
                r->stats.min_ns, r->stats.max_ns, r->stats.mean_ns, r->stats.median_ns,
                r->stats.stddev_ns, r->stats.p95_ns, r->stats.p99_ns,
                (i < g_bench.result_count - 1) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
    if (g_bench.config.verbose)
        printf("Results written to %s\n", filename);
    return 0;
}

void bench_cleanup(void) { memset(&g_bench, 0, sizeof(g_bench)); }
