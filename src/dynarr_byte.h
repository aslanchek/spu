/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef DYNAMIC_ARRAY_BYTE_HEADER
#define DYNAMIC_ARRAY_BYTE_HEADER

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t capacity;
    size_t size;

    uint8_t *arr;
} dynarr_byte;

dynarr_byte dynarr_byte_init(size_t n) {
    return (dynarr_byte) {
        .capacity = n,
        .size = 0,
        .arr = calloc(n, sizeof(uint8_t))
    };
}

void dynarr_byte_destroy(dynarr_byte *dnrr) {
    dnrr->capacity = 0;
    dnrr->size = 0;
    free(dnrr->arr);
    dnrr->arr = NULL;
}

static void _dynarr_byte_resize(dynarr_byte *dnrr, size_t newsize) {
    dnrr->arr = reallocarray(dnrr->arr, newsize, sizeof(uint8_t));
    dnrr->capacity = newsize;
}

static size_t dynarr_byte_size(dynarr_byte *dnrr) {
    return dnrr->size;
}

static void dynarr_byte_insert(dynarr_byte *dnrr, const void *val, size_t sz, size_t pos) {
    if ( dnrr->size + sz > dnrr->capacity) {
        _dynarr_byte_resize(dnrr, 2 * (dnrr->size + sz));
    }

    for (size_t i = 0; i < sz; i++) {
      dnrr->arr[pos + i] = ((const uint8_t *)val) [i];
    }
};


static void dynarr_byte_append(dynarr_byte *dnrr, const void *val, size_t sz) {
    if ( dnrr->size + sz > dnrr->capacity) {
        _dynarr_byte_resize(dnrr, 2 * (dnrr->size + sz));
    }

    for (size_t i = 0; i < sz; i++) {
      dnrr->arr[dnrr->size++] = ((const uint8_t *)val) [i];
    }
};

static uint8_t dynarr_byte_accss(dynarr_byte *dnrr, size_t i) {
    return dnrr->arr[i];
};

#endif

