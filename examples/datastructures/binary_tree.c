#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"
#include <stdlib.h>

typedef struct node
{
    int data;
    struct node *left, *right;
} node_t;

static node_t *node_new(int d)
{
    node_t *n = malloc(sizeof(node_t));
    n->data = d;
    n->left = n->right = NULL;
    return n;
}

static node_t *bst_insert(node_t *n, int d)
{
    if (!n)
        return node_new(d);
    if (d <= n->data)
        n->left = bst_insert(n->left, d);
    else
        n->right = bst_insert(n->right, d);
    return n;
}

static node_t *bst_find(node_t *n, int d)
{
    while (n && n->data != d)
        n = (d < n->data) ? n->left : n->right;
    return n;
}

static void bst_free(node_t *n)
{
    if (!n)
        return;
    bst_free(n->left);
    bst_free(n->right);
    free(n);
}

static int g_keys[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 90};
#define NKEYS (sizeof(g_keys) / sizeof(g_keys[0]))

BENCH(bst_insert_15)
{
    node_t *root = NULL;
    for (size_t i = 0; i < NKEYS; i++)
        root = bst_insert(root, g_keys[i]);
    KEEP(root);
    bst_free(root);
}

BENCH(bst_find_hit)
{
    node_t *root = NULL;
    for (size_t i = 0; i < NKEYS; i++)
        root = bst_insert(root, g_keys[i]);
    node_t *r = bst_find(root, 45);
    KEEP(r);
    bst_free(root);
}

BENCH(bst_find_miss)
{
    node_t *root = NULL;
    for (size_t i = 0; i < NKEYS; i++)
        root = bst_insert(root, g_keys[i]);
    node_t *r = bst_find(root, 999);
    KEEP(r);
    bst_free(root);
}

int main(void) { return bench_main(); }
