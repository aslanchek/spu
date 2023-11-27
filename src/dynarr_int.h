/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef DYNAMIC_ARRAY_INT_HEADER
#define DYNAMIC_ARRAY_INT_HEADER

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t capacity;
    size_t size;

    int *arr;
} dynarr_int;

dynarr_int dynarr_int_init(size_t n) {
    return (dynarr_int) {
        .capacity = n,
        .size = 0,
        .arr = calloc(n, sizeof(int))
    };
}

void dynarr_int_destroy(dynarr_int *dnrr) {
    dnrr->capacity = 0;
    dnrr->size = 0;
    free(dnrr->arr);
    dnrr->arr = NULL;
}

static void _dynarr_int_resize(dynarr_int *dnrr, size_t newsize) {
    dnrr->arr = reallocarray(dnrr->arr, newsize, sizeof(int));
    dnrr->capacity = newsize;
}

static size_t dynarr_int_size(dynarr_int *dnrr) {
    return dnrr->size;
}

static void dynarr_int_append(dynarr_int *dnrr, const int val) {
    if ( dnrr->size == dnrr->capacity ) {
        _dynarr_int_resize(dnrr, 2 * dnrr->capacity);
    }

    dnrr->arr[dnrr->size++] = val;
};

static int dynarr_int_accss(dynarr_int *dnrr, size_t i) {
    return dnrr->arr[i];
};

#endif

