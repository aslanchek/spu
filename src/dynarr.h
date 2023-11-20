#include <stdint.h>
#include <stdlib.h>

typedef uint8_t TYPE;

typedef struct {
    size_t capacity;
    size_t size;

    TYPE *arr;
} dynarr;

dynarr dynarr_init(size_t n) {
    return (dynarr) {
        .capacity = n,
        .size = 0,
        .arr = calloc(n, sizeof(TYPE))
    };
}

void dynarr_destroy(dynarr *dnrr) {
    dnrr->capacity = 0;
    dnrr->size = 0;
    free(dnrr->arr);
    dnrr->arr = NULL;
}

static void _dynarr_resize(dynarr *dnrr, size_t newsize) {
    dnrr->arr = reallocarray(dnrr->arr, newsize, sizeof(TYPE));
    dnrr->capacity = newsize;
}

static size_t dynarr_size(dynarr *dnrr) {
    return dnrr->size;
}

static void dynarr_append(dynarr *dnrr, const void *val, size_t sz) {
    if (dnrr->size == dnrr->capacity) {
        _dynarr_resize(dnrr, dnrr->capacity * 2);
    }

    for (size_t i = 0; i < sz; i++) {
      dnrr->arr[dnrr->size++] = ((const uint8_t *)val) [i];
    }
};

static TYPE dynarr_accss(dynarr *dnrr, size_t i) {
    return dnrr->arr[i];
};

