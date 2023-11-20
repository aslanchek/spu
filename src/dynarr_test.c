#include <stdint.h>
#include <stdio.h>
#include "dynarr.h"

int main() {
    dynarr aaa = dynarr_init(5);

    printf("size = %zu\n", dynarr_size(&aaa));
    for (size_t i = 0; i < dynarr_size(&aaa); i++) {
        printf("[%03zu] 0x%02x\n", i, dynarr_accss(&aaa, i));
    }

    dynarr_append(&aaa, (uint8_t []) { 0x23, 0x33, 0x66 }, 3);

    printf("size = %zu\n", dynarr_size(&aaa));
    for (size_t i = 0; i < dynarr_size(&aaa); i++) {
        printf("[%03zu] 0x%02x\n", i, dynarr_accss(&aaa, i));
    }

    int a = 1;

    dynarr_append(&aaa, &a, sizeof(a));

    a = 1337;
    dynarr_append(&aaa, &a, sizeof(a));

    printf("size = %zu\n", dynarr_size(&aaa));
    for (size_t i = 0; i < dynarr_size(&aaa); i++) {
        printf("[%03zu] 0x%02x\n", i, dynarr_accss(&aaa, i));
    }

    dynarr_destroy(&aaa);
    
    return 0;
}

