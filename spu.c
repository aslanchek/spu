#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

#include "third-party/stack/stack.h"

DEFINE_STACK(int, "%d");
DEFINE_STACK(double, "%lf");

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YLW "\x1b[33m"
#define ANSI_COLOR_RST "\x1b[0m"
#define RED(str) ANSI_COLOR_RED str ANSI_COLOR_RST
#define LOGMETA __FILE__, __LINE__, __PRETTY_FUNCTION__
#define NOLOGMETA NULL, -1, NULL 
#define PRETTY_ERROR(issue) fprintf(stderr, RED("[SPU error]")" %s:%d in (%s): %s: %s\n", LOGMETA, issue, strerror(errno)); exit(EXIT_FAILURE);
#define PRETTY_FAIL(issue)  fprintf(stderr, RED("[SPU error]")" %s:%d in (%s): %s\n", LOGMETA, issue); exit(EXIT_FAILURE);

#define PRETTY_LOG_BUFFER_SIZE 2048

void PRETTY_LOG(const char *filename, const ssize_t fileline, const char* function, const char *fmt, ...) {
    (void)filename;
    (void)fileline;
    (void)function;
    (void)fmt;
#ifdef VERBOSE
    va_list args;
    va_start(args, fmt);

    char buffer[PRETTY_LOG_BUFFER_SIZE] = {};

    vsprintf(buffer, fmt, args);

    if (filename && fileline != -1 && function) {
        fprintf(stderr, "[SPU log] %s:%zu in (%s): %s\n", filename, (size_t)fileline, function, buffer);
    } else {
        fprintf(stderr, "[SPU log] %s\n", buffer);
    }

    va_end(args);
#endif
}

long int fsize(int fildes) {
    struct stat meta;

    if (fstat(fildes, &meta) < 0) {
      return -1;
    }

    return meta.st_size;
}

typedef struct {
    stack_int stack;
    char *text;
    size_t textsize;
    size_t cc; // command counter
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
    spu->text = NULL;
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
 *  +-----TEXT-----
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
 *  +--------------
 * total text size = 134
 */
void SPU_dump(SPU *spu) {
    STACK_DUMP(&spu->stack, int);

    fprintf(stderr, "+-----TEXT-----\n");

    fprintf(stderr, "+--------------\n"
                    "total text size = %zu\n", spu->textsize);
}

int SPU_loadtext(SPU *spu, char *filename) {
    // reads programm text from filename
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("open");
    }

    ssize_t size = fsize(fd);
    if (size < 0) {
        PRETTY_ERROR("Faulty reading file stat");
    }

    spu->textsize = (size_t)size;
    
    spu->text = calloc(spu->textsize + 1, 1);

    read(fd, spu->text, spu->textsize);

    close(fd);
    return 0;
}

/*
 *
 * in
 * out
 *
 * add
 * mul
 * sqrt
 * sin
 * cos
 *
 *
 * push
 * pop
 *
 *
 */

typedef enum {
    HLT  = 0,
    PUSH = 1,
    POP  = 2,
    OUT  = 3,
    IN   = 4,
    ADD  = 5,
    SUB  = 6,
    MUL  = 7,
} COMMANDS;

char *STRCOMMANDS[] = {
    /*0*/"hlt" ,
    /*1*/"push",
    /*2*/"pop" ,
    /*3*/"out" ,
    /*4*/"in"  ,
    /*5*/"add" ,
    /*6*/"sub" ,
    /*7*/"mul" ,
};

int SPU_run(SPU *spu) {
    assert(!stack_int_validate(&spu->stack));
    assert(spu->text);
    assert(spu->textsize != 0);
    assert(spu->cc == 0);
    
    PRETTY_LOG(NOLOGMETA, "Running text...");

    while (spu->cc < spu->textsize) {
        if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[HLT], strlen(STRCOMMANDS[HLT])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[HLT]);
            spu->cc += strlen(STRCOMMANDS[HLT]);

            // action
            break;
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[PUSH], strlen(STRCOMMANDS[PUSH])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[PUSH]);
            spu->cc += strlen(STRCOMMANDS[PUSH]);

            // action
            // checking text integrity
            if (*(spu->text + spu->cc) != ' ') {
                PRETTY_LOG(NOLOGMETA, "\"%s\" parse error", STRCOMMANDS[PUSH], *(spu->text + spu->cc));
                return -1;
            }
            int arg = atoi(spu->text + spu->cc + 1);
            stack_int_push(&spu->stack, arg IF_VERBOSE(, LOGMETA));
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[POP], strlen(STRCOMMANDS[POP])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[POP]);

            // action
            spu->cc += strlen(STRCOMMANDS[POP]);
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[OUT], strlen(STRCOMMANDS[OUT])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[OUT]);

            // action
            spu->cc += strlen(STRCOMMANDS[OUT]);
            int top = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            fprintf(stdout, "%d\n", top);
            //
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[IN], strlen(STRCOMMANDS[IN])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[IN]);
            spu->cc += strlen(STRCOMMANDS[IN]);

            // action
            int tmp = 0;
            scanf("%d", &tmp);
            stack_int_push(&spu->stack, tmp IF_VERBOSE(, LOGMETA));
            //
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[ADD], strlen(STRCOMMANDS[ADD])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[ADD]);
            spu->cc += strlen(STRCOMMANDS[ADD]);

            // action
            int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_push(&spu->stack, op1+op2 IF_VERBOSE(, LOGMETA));
            //
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[SUB], strlen(STRCOMMANDS[SUB])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[SUB]);
            spu->cc += strlen(STRCOMMANDS[SUB]);

            // action
            int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_push(&spu->stack, op2-op1 IF_VERBOSE(, LOGMETA));
            //
        } else if ( !strncmp(spu->text + spu->cc, STRCOMMANDS[MUL], strlen(STRCOMMANDS[MUL])) ) {
            PRETTY_LOG(NOLOGMETA, "\"%s\" encountered", STRCOMMANDS[MUL]);
            spu->cc += strlen(STRCOMMANDS[MUL]);

            // action
            int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
            stack_int_push(&spu->stack, op1*op2 IF_VERBOSE(, LOGMETA));
            //
        } else {
            spu->cc++;
        }
    }

    PRETTY_LOG(NOLOGMETA, "Halting...");

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRETTY_LOG(NOLOGMETA, "Usage: %s [FILE]", *argv);
        PRETTY_FAIL("Invalid agruments");
    }
    argv++;

    SPU main_spu = SPU_init();

    SPU_loadtext(&main_spu, *argv);

    SPU_run(&main_spu);

    SPU_destroy(&main_spu);
    return 0;
}

