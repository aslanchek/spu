/*********************************
 * Software processing unit
 *
 *********************************/
#ifndef SOFTWARE_PROCESSING_UNIT_HEADER_H
#define SOFTWARE_PROCESSING_UNIT_HEADER_H

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#define SIG (uint64_t []) { 0xBADC0FFE00000001U }

#include "../third-party/stack/stack.h"
#include "log.h"
#include "is.h"

long int fsize(int fildes) {
    struct stat meta;
    if (fstat(fildes, &meta) < 0) {
      return -1;
    }

    return meta.st_size;
}

DEFINE_STACK(int, "%d");

typedef struct {
    stack_int stack;    // stack managing integers
    uint8_t *text;      // pointer to executable text
    size_t textsize;    // size of text (= size of input file)
    size_t pc;          // program counter
    int regs[SIZEOFARR(REGS_SET)];
} SPU;

SPU SPU_init();

void SPU_destroy(SPU *spu);

void SPU_dump(SPU *spu);

int SPU_loadtext(SPU *spu, char *filename);

command_t _parse_cmd(uint8_t *c);

int SPU_run(SPU *spu);

#endif

