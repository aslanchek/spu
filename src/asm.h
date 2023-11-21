#ifndef ASSEMBLER_HEADER_H
#define ASSEMBLER_HEADER_H

#define SIG (uint64_t []) { 0x0101010101010101 }

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "log.h"
#include "is.h"
#include "dynarr.h"

static long unsigned int finode(const char *filename) {
    struct stat meta;
    if (lstat(filename, &meta) < 0) {
      return 0;
    }

    return meta.st_ino;
}


static long int fsize(int fildes) {
    struct stat meta;
    if (fstat(fildes, &meta) < 0) {
      return -1;
    }

    return meta.st_size;
}

typedef struct {
    char *text;           // pointer to executable text
    size_t textsize;      // size of text (= size of input file)
    char **tokens;        // tokens
    size_t numtok;        // number of tokens
    size_t tc;            // token counter
    dynarr bytecode;      // translated byte code
} assembler;

assembler assembler_init();

void assembler_destroy(assembler *spu);

void assembler_dump(assembler *spu);

int assembler_loadtext(assembler *spu, char *source);

command_t _parse_cmd(char *strline);

int assembler_translate(assembler *spu, const char *output);

#endif

