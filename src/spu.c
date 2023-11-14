#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "../third-party/stack/stack.h"

#include "log.h"
#include "is.h"

DEFINE_STACK(int, "%d");
DEFINE_STACK(double, "%lf");

long int fsize(int fildes) {
    struct stat meta;
    if (fstat(fildes, &meta) < 0) {
      return -1;
    }

    return meta.st_size;
}

typedef struct {
    stack_int stack; // stack managing integers
    char *text;      // pointer to executable text
    size_t textsize; // size of text (= size of input file)
    size_t cc;       // command counter
} SPU;

SPU SPU_init() {
    SPU new = {
       .stack    = stack_int_init(),
       .text     = NULL,
       .textsize = 0,
       .cc       = 0,
    };

    return new;
}

void SPU_destroy(SPU *spu) {
    stack_int_destroy(&spu->stack);
    free(spu->text);

    bzero(spu, sizeof(SPU));
}

/*
 * [stack log] spu.c:84 in (SPU_dump): stack_dump()
 * ---STACK DUMP---
 * STATUS: OK
 * stack<int> [0x7ffcd1da9430] "&spu->stack" {
 *   capacity = 2
 *   size     = 1
 *   data [0x5605256f22a0] {
 *     *[0] = -23
 *      [1] = 32
 *   }
 * }
 *  +------TEXT------
 *  | in       ;b
 *  | in       ;b
 *  | mul      ;b^2
 * cc>push 4
 *  | in       ;a
 *  | in       ;c
 *  | mul      ;ac
 *  | mul      ;4ac
 *  | sub      ;b^2 - 4ac
 *  | out      ;prt
 *  | htl
 *  +----------------
 * total text size = 134
 */
void SPU_dump(SPU *spu) {
    STACK_DUMP(&spu->stack, int);

    fprintf(stderr, "+------TEXT------\n");

    fprintf(stderr, "%s", spu->text);

    fprintf(stderr, "+----------------\n"
                    "total text size = %zu\n", spu->textsize);
}

int SPU_loadtext(SPU *spu, char *filename) {
    // reads programm text from filename
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("spu", "open()");
    }

    ssize_t size = fsize(fd);
    if (size < 0) {
        PRETTY_ERROR("spu", "Faulty reading file stat");
    }

    spu->textsize = (size_t)size;
    
    spu->text = calloc(spu->textsize + 1, 1);

    read(fd, spu->text, spu->textsize);

    close(fd);
    return 0;
}


/*
 * parses arbitrary string
 *  [push 123\n]      -> push
 *  [pop\n]           -> pop
 *  [addafadsfadsf\n] -> add
 *  [3.14push\n]      -> unparsed
 *
 */
command_t _parse_cmd(char *strline) {
    for(size_t i = 0; i < SIZEOFARR(INSTRCTN_SET); i++) {
        if(strncasecmp(INSTRCTN_SET[i].name, strline, strlen(INSTRCTN_SET[i].name)) == 0) {
            PRETTY_LOG("spu", NOLOGMETA, "\"%s\" encountered", INSTRCTN_SET[i].name);
            return INSTRCTN_SET[i];
        }
    }

    return /*command: none*/COMMAND_NONE;
}


#define GENERATE_COMMAND(TXT, OPCODE, ...)\
    case COMMANDS_##TXT:\
        __VA_ARGS__\
        break;\

int SPU_run(SPU *spu) {
    assert(!stack_int_validate(&spu->stack));
    assert(spu->text);
    assert(spu->textsize != 0);
    assert(spu->cc == 0);
    
    PRETTY_LOG("spu", NOLOGMETA, "Running text...");

    while (spu->cc < spu->textsize) {
        char *commandstr = (spu->text + spu->cc);
        command_t toexecute = _parse_cmd(commandstr);

        switch (toexecute.code) {
            #include "is.dsl"
            default:
                break;
        }

        spu->cc += toexecute.len;
    }

    return 0;
}

#undef GENERATE_COMMAND


int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRETTY_LOG("spu", NOLOGMETA, "Usage: %s [FILE]", *argv);
        PRETTY_FAIL("spu", "Invalid agruments");
    }
    argv++;

    SPU main_spu = SPU_init();

    SPU_loadtext(&main_spu, *argv);

    int ret = SPU_run(&main_spu);
    if (ret) {
        PRETTY_LOG("spu", NOLOGMETA, RED("interpeter error"));
        SPU_dump(&main_spu);
    }

    SPU_destroy(&main_spu);
    return 0;
}

