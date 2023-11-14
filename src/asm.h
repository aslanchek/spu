#ifndef ASSEMBLER_HEADER_H
#define ASSEMBLER_HEADER_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../third-party/stack/stack.h"

#include "log.h"
#include "is.h"

static long int fsize(int fildes) {
    struct stat meta;
    if (fstat(fildes, &meta) < 0) {
      return -1;
    }

    return meta.st_size;
}


/*
 *
 * lines  text
 * V       V
 * l[0] ->|push 5
 * l[1] ->|pop
 * l[2] ->|hlt
 *
 */
typedef struct {
    char *text;          // pointer to executable text
    size_t textsize;     // size of text (= size of input file)
    char **tokens;       // lines
    size_t numtok;       // size of text (= size of input file)
    size_t tc;           // token counter
    uint8_t *bytecode;   // translated byte code
} assembler;

assembler assembler_init();

void assembler_destroy(assembler *spu);

void assembler_dump(assembler *spu);

int assembler_loadtext(assembler *spu, char *source);

command_t _parse_cmd(char *strline);

int assembler_translate(assembler *spu);

#endif

