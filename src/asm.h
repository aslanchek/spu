/*********************************
 * Assembler header
 *
 *********************************/
#ifndef ASSEMBLER_HEADER_H
#define ASSEMBLER_HEADER_H

#define SIG (uint64_t []) { 0xBADC0FFE00000002U }

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "log.h"
#include "is.h"
#include "dynarr_byte.h"
#include "dynarr_int.h"

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
    char        *text;      // pointer to executable text
    size_t      textsize;   // size of text (= size of input file)
    char      **tokens;     // tokens
    size_t      numtok;     // number of tokens
    size_t      tc;         // token counter
    dynarr_byte bytecode;   // translated byte code
    label_t    *labels;
    size_t      numlabels;
} assembler;

assembler assembler_init();

void assembler_destroy(assembler *ass);

void assembler_dump(assembler *ass);

// stage 0
int assembler_loadtext(assembler *ass, char *source);

arg_t _parse_arg(char *token, assembler *ass);
command_t _parse_cmd(char *token);

// stage 1
int assembler_translate(assembler *ass);

// stage 2
int assembler_link(assembler *ass);

// stage 3
int assembler_write(assembler *ass, const char *output);

#endif

