#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"
#include <stdlib.h>

#define SMALL_N   100
#define MEDIUM_N  1000
#define LARGE_N   10000

static int g_small[SMALL_N], g_medium[MEDIUM_N], g_large[LARGE_N];

static void init_sorted(int *arr, int n) { for (int i = 0; i < n; i++) arr[i] = i * 2; }

static int linear_search(const int *arr, int n, int target) {
    for (int i = 0; i < n; i++) if (arr[i] == target) return i;
    return -1;
}

static int binary_search(const int *arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

static int interpolation_search(const int *arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi && target >= arr[lo] && target <= arr[hi]) {
        if (lo == hi) return (arr[lo] == target) ? lo : -1;
        int pos = lo + ((long)(target - arr[lo]) * (hi - lo)) / (arr[hi] - arr[lo]);
        if (arr[pos] == target) return pos;
        if (arr[pos] < target) lo = pos + 1; else hi = pos - 1;
    }
    return -1;
}

static int jump_search(const int *arr, int n, int target) {
    int step = 1; while (step * step < n) step++;
    int prev = 0, curr = step;
    while (curr < n && arr[curr] < target) { prev = curr; curr += step; }
    for (int i = prev; i < n && i <= curr; i++) if (arr[i] == target) return i;
    return -1;
}

BENCH(linear_100, "linear 100") { init_sorted(g_small, SMALL_N); volatile int r = linear_search(g_small, SMALL_N, 98); KEEP(r); }
BENCH(binary_100, "binary 100") { init_sorted(g_small, SMALL_N); volatile int r = binary_search(g_small, SMALL_N, 98); KEEP(r); }
BENCH(interp_100, "interp 100") { init_sorted(g_small, SMALL_N); volatile int r = interpolation_search(g_small, SMALL_N, 98); KEEP(r); }
BENCH(jump_100, "jump 100") { init_sorted(g_small, SMALL_N); volatile int r = jump_search(g_small, SMALL_N, 98); KEEP(r); }

BENCH(linear_1k, "linear 1k") { init_sorted(g_medium, MEDIUM_N); volatile int r = linear_search(g_medium, MEDIUM_N, 1998); KEEP(r); }
BENCH(binary_1k, "binary 1k") { init_sorted(g_medium, MEDIUM_N); volatile int r = binary_search(g_medium, MEDIUM_N, 1998); KEEP(r); }
BENCH(interp_1k, "interp 1k") { init_sorted(g_medium, MEDIUM_N); volatile int r = interpolation_search(g_medium, MEDIUM_N, 1998); KEEP(r); }

BENCH(linear_10k, "linear 10k") { init_sorted(g_large, LARGE_N); volatile int r = linear_search(g_large, LARGE_N, 19998); KEEP(r); }
BENCH(binary_10k, "binary 10k") { init_sorted(g_large, LARGE_N); volatile int r = binary_search(g_large, LARGE_N, 19998); KEEP(r); }
BENCH(interp_10k, "interp 10k") { init_sorted(g_large, LARGE_N); volatile int r = interpolation_search(g_large, LARGE_N, 19998); KEEP(r); }

BENCH(linear_miss, "linear 1k miss") { init_sorted(g_medium, MEDIUM_N); volatile int r = linear_search(g_medium, MEDIUM_N, 1999); KEEP(r); }
BENCH(binary_miss, "binary 1k miss") { init_sorted(g_medium, MEDIUM_N); volatile int r = binary_search(g_medium, MEDIUM_N, 1999); KEEP(r); }

int main(void) { return bench_main(); }

