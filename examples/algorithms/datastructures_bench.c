#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"
#include <stdlib.h>
#include <string.h>

#define N 1000

typedef struct node
{
  int data;
  struct node *next;
} node_t;
static node_t g_pool[N];
static int g_pool_idx;

static node_t *list_alloc(int d)
{
  node_t *n = &g_pool[g_pool_idx++];
  n->data = d;
  n->next = NULL;
  return n;
}
static node_t *list_prepend(node_t *h, int d)
{
  node_t *n = list_alloc(d);
  n->next = h;
  return n;
}
static node_t *list_append(node_t *h, int d)
{
  node_t *n = list_alloc(d);
  if (!h)
    return n;
  node_t *c = h;
  while (c->next)
    c = c->next;
  c->next = n;
  return h;
}
static int list_find(node_t *h, int d)
{
  int i = 0;
  for (node_t *n = h; n; n = n->next, i++)
    if (n->data == d)
      return i;
  return -1;
}

typedef struct
{
  int *data;
  size_t size, cap;
} vec_t;
static void vec_init(vec_t *v)
{
  v->data = malloc(16 * sizeof(int));
  v->size = 0;
  v->cap = 16;
}
static void vec_push(vec_t *v, int x)
{
  if (v->size >= v->cap)
  {
    v->cap *= 2;
    v->data = realloc(v->data, v->cap * sizeof(int));
  }
  v->data[v->size++] = x;
}
static void vec_insert(vec_t *v, size_t i, int x)
{
  if (v->size >= v->cap)
  {
    v->cap *= 2;
    v->data = realloc(v->data, v->cap * sizeof(int));
  }
  memmove(&v->data[i + 1], &v->data[i], (v->size - i) * sizeof(int));
  v->data[i] = x;
  v->size++;
}
static void vec_free(vec_t *v) { free(v->data); }

static int g_arr[N];
static vec_t g_vec;

BENCH(array_write_1k)
{
  for (int i = 0; i < N; i++)
    g_arr[i] = i;
  KEEP(g_arr);
}
BENCH(array_read_1k)
{
  for (int i = 0; i < N; i++)
    g_arr[i] = i;
  volatile int s = 0;
  for (int i = 0; i < N; i++)
    s += g_arr[i];
  KEEP(s);
}
BENCH(array_rand_1k)
{
  for (int i = 0; i < N; i++)
    g_arr[i] = i;
  volatile int s = 0;
  for (int i = 0; i < N; i++)
    s += g_arr[(i * 7) % N];
  KEEP(s);
}

BENCH(vec_push_1k)
{
  vec_init(&g_vec);
  for (int i = 0; i < N; i++)
    vec_push(&g_vec, i);
  KEEP(g_vec);
  vec_free(&g_vec);
}
BENCH(vec_ins_front_100)
{
  vec_init(&g_vec);
  for (int i = 0; i < 100; i++)
    vec_insert(&g_vec, 0, i);
  KEEP(g_vec);
  vec_free(&g_vec);
}

BENCH(list_prepend_1k)
{
  g_pool_idx = 0;
  node_t *h = NULL;
  for (int i = 0; i < N; i++)
    h = list_prepend(h, i);
  KEEP(h);
}
BENCH(list_append_100)
{
  g_pool_idx = 0;
  node_t *h = NULL;
  for (int i = 0; i < 100; i++)
    h = list_append(h, i);
  KEEP(h);
}
BENCH(list_traverse_1k)
{
  g_pool_idx = 0;
  node_t *h = NULL;
  for (int i = 0; i < N; i++)
    h = list_prepend(h, i);
  volatile int s = 0;
  for (node_t *n = h; n; n = n->next)
    s += n->data;
  KEEP(s);
}
BENCH(list_find_last)
{
  g_pool_idx = 0;
  node_t *h = NULL;
  for (int i = 0; i < N; i++)
    h = list_prepend(h, i);
  volatile int r = list_find(h, 0);
  KEEP(r);
}
BENCH(array_find_last)
{
  for (int i = 0; i < N; i++)
    g_arr[i] = N - 1 - i;
  volatile int r = -1;
  for (int i = 0; i < N; i++)
    if (g_arr[i] == 0)
    {
      r = i;
      break;
    }
  KEEP(r);
}

int main(void) { return bench_main(); }
