#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"
#include <stdlib.h>
#include <string.h>

#define SMALL_N 100
#define MEDIUM_N 1000
#define LARGE_N 10000

static int g_small[SMALL_N], g_medium[MEDIUM_N], g_large[LARGE_N];

static void fill_reverse(int *arr, int n)
{
    for (int i = 0; i < n; i++)
        arr[i] = n - i;
}
static void fill_random(int *arr, int n)
{
    for (int i = 0; i < n; i++)
        arr[i] = rand();
}
static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void insertion_sort(int *arr, int n)
{
    for (int i = 1; i < n; i++)
    {
        int key = arr[i], j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static void merge(int *arr, int *tmp, int l, int m, int r)
{
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r)
        tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    while (i <= m)
        tmp[k++] = arr[i++];
    while (j <= r)
        tmp[k++] = arr[j++];
    for (i = l; i <= r; i++)
        arr[i] = tmp[i];
}

static void merge_sort_impl(int *arr, int *tmp, int l, int r)
{
    if (l >= r)
        return;
    int m = l + (r - l) / 2;
    merge_sort_impl(arr, tmp, l, m);
    merge_sort_impl(arr, tmp, m + 1, r);
    merge(arr, tmp, l, m, r);
}

static void merge_sort(int *arr, int n)
{
    int *tmp = malloc(n * sizeof(int));
    merge_sort_impl(arr, tmp, 0, n - 1);
    free(tmp);
}
static void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static void heapify(int *arr, int n, int i)
{
    int largest = i, l = 2 * i + 1, r = 2 * i + 2;
    if (l < n && arr[l] > arr[largest])
        largest = l;
    if (r < n && arr[r] > arr[largest])
        largest = r;
    if (largest != i)
    {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}

static void heap_sort(int *arr, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);
    for (int i = n - 1; i > 0; i--)
    {
        swap(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}

static int partition(int *arr, int lo, int hi)
{
    int pivot = arr[hi], i = lo - 1;
    for (int j = lo; j < hi; j++)
        if (arr[j] < pivot)
            swap(&arr[++i], &arr[j]);
    swap(&arr[i + 1], &arr[hi]);
    return i + 1;
}

static void quicksort_impl(int *arr, int lo, int hi)
{
    if (lo >= hi)
        return;
    int p = partition(arr, lo, hi);
    quicksort_impl(arr, lo, p - 1);
    quicksort_impl(arr, p + 1, hi);
}

static void quicksort(int *arr, int n) { quicksort_impl(arr, 0, n - 1); }

BENCH(qsort_100_rev)
{
    fill_reverse(g_small, SMALL_N);
    qsort(g_small, SMALL_N, sizeof(int), cmp_int);
    KEEP(g_small);
}
BENCH(insertion_100_rev)
{
    fill_reverse(g_small, SMALL_N);
    insertion_sort(g_small, SMALL_N);
    KEEP(g_small);
}
BENCH(merge_100_rev)
{
    fill_reverse(g_small, SMALL_N);
    merge_sort(g_small, SMALL_N);
    KEEP(g_small);
}
BENCH(heap_100_rev)
{
    fill_reverse(g_small, SMALL_N);
    heap_sort(g_small, SMALL_N);
    KEEP(g_small);
}
BENCH(quick_100_rev)
{
    fill_reverse(g_small, SMALL_N);
    quicksort(g_small, SMALL_N);
    KEEP(g_small);
}

BENCH(qsort_1k_rand)
{
    fill_random(g_medium, MEDIUM_N);
    qsort(g_medium, MEDIUM_N, sizeof(int), cmp_int);
    KEEP(g_medium);
}
BENCH(merge_1k_rand)
{
    fill_random(g_medium, MEDIUM_N);
    merge_sort(g_medium, MEDIUM_N);
    KEEP(g_medium);
}
BENCH(heap_1k_rand)
{
    fill_random(g_medium, MEDIUM_N);
    heap_sort(g_medium, MEDIUM_N);
    KEEP(g_medium);
}
BENCH(quick_1k_rand)
{
    fill_random(g_medium, MEDIUM_N);
    quicksort(g_medium, MEDIUM_N);
    KEEP(g_medium);
}

int main(void)
{
    srand(42);
    return bench_main();
}
