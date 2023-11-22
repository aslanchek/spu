#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "asm.h"
#include "is.h"
#include "log.h"

assembler assembler_init() {
    assembler new = {
        .text        = NULL,
        .textsize    = 0,
        .tokens      = NULL,
        .numtok      = 0,
        .tc          = 0,
        .bytecode    = dynarr_init(1),
    };

    return new;
}

void assembler_destroy(assembler *ass) {
    assert(ass->text);
    assert(ass->tokens);
    assert(ass->bytecode.arr);
    assert(ass->tc == ass->numtok);

    free(ass->text);
    free(ass->tokens);

    dynarr_destroy(&ass->bytecode);

    bzero(ass, sizeof(assembler));
}


void assembler_dump(assembler *ass) {
    fprintf(stderr, "+---- ASM DUMP -----\n");
    fprintf(stderr, "+--- TOKENS ---\n");

    for (size_t i = 0; i < ass->tc; i++) {
        fprintf(stderr, "  [%05zu] %s\n", i, ass->tokens[i]);
    }

    if (ass->tc < ass->numtok) {
        fprintf(stderr, "->[%05zu] %s\n", ass->tc, ass->tokens[ass->tc]);
    }

    for (size_t i = ass->tc + 1; i < ass->numtok; i++) {
        fprintf(stderr, "  [%05zu] %s\n", i, ass->tokens[i]);
    }

    fprintf(stderr, "+--------------\n"
                    "total tokens number = %zu\n", ass->numtok);

    fprintf(stderr, "+--- BYTECODE ---\n");

    for (size_t i = 0; i < dynarr_size(&ass->bytecode); i++) {
        fprintf(stderr, "[%03zu] 0x%02x          ""0b"
                BYTE2BIN_PATTERN " \n", i,
                dynarr_accss(&ass->bytecode, i),
                BYTE2BIN(dynarr_accss(&ass->bytecode, i)));
    }

    fprintf(stderr, "\n");

    fprintf(stderr, "+----------------\n");

}

/*
 * lexer
 *
 * text = [ push 5\npush rax\nadd\nhlt ] -> [ "push", "5", "push", "rax", "add", "hlt" ]
 *
 */
int assembler_loadtext(assembler *ass, char *source) {
    assert(ass->text         == NULL);
    assert(ass->textsize     == 0);
    assert(ass->tokens       == NULL);
    assert(ass->numtok       == 0);
    // TODO: validate dynarr

    // - reads programm text from filename -------------------
    int fd = open(source, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("assembler", LOGMETA, "open()");
        return 1;
    }

    ssize_t size = fsize(fd);
    if (size < 0) {
        PRETTY_ERROR("assembler", LOGMETA, "Faulty reading file stat");
        return 1;
    }

    ass->textsize = (size_t)size;
    
    ass->text = calloc(ass->textsize + 1, 1);

    read(fd, ass->text, ass->textsize);

    close(fd);

    // - tokenizing ------------------------------------------
    
    // eliminating commentaries
    for (char *ptr = ass->text; ptr < ass->text + ass->textsize; ptr++) {
        if (*ptr == ';') while (*ptr != '\n') { *ptr++= ' '; }
    }
    
    // getting tokens number
    for (size_t i = 0; i < ass->textsize; i++) {
        if (!isspace(ass->text[i])) {
            ass->numtok++;
            while (!isspace(ass->text[++i]));
        }
    }

    ass->tokens = calloc(ass->numtok, sizeof(char *));

    char delims[] = " \n\t";

    size_t nt = 0;
    char *token = strtok(ass->text, delims);
    while (token) {
        ass->tokens[nt++] = token;
        token = strtok(NULL, delims);
    }

    return 0;
}


command_t _parse_cmd(char *token) {
    for(size_t i = 0; i < SIZEOFARR(INSTRCTN_SET); i++) {
        if(strncasecmp(INSTRCTN_SET[i].name, token, strlen(INSTRCTN_SET[i].name)) == 0) {
            PRETTY_LOG("assembler", NOLOGMETA, "\"%s\" encountered", INSTRCTN_SET[i].name);
            return INSTRCTN_SET[i];
        }
    }

    return /*failed to parse*/COMMAND_NONE;
}

arg_t _parse_arg(char *token) {
    // check if arg is register
    for(size_t i = 0; i < SIZEOFARR(REGS_SET); i++) {
        if (strncasecmp(REGS_SET[i].name, token, strlen(REGS_SET[i].name)) == 0) {
            PRETTY_LOG("assembler", NOLOGMETA, "\"%s\" encountered", REGS_SET[i].name);
            return (const arg_t) { .type = ARG_TYPE_REG, .regarg = REGS_SET[i] };
        }
    }

    // check if arg is integer imm
    char *endptr = NULL;
    long arg = strtol(token, &endptr, 10);

    if (endptr == strchr(token, '\0') && arg < INT_MAX && arg > INT_MIN ) {
        return (arg_t) { .type = ARG_TYPE_INT, .intarg = (int)arg };
    }

    return /*failed to parse*/ARG_NONE;
}

#define WRITE_CMD(ARGN)\
if (ARGN) {\
    const arg_t arg = _parse_arg(ass->tokens[ass->tc++]);\
    switch ( arg.type ) {\
        case ARG_TYPE_INT: {\
            dynarr_append(&ass->bytecode, (uint8_t []) { toexecute.opcode | 0b00100000 }, 1);\
            dynarr_append(&ass->bytecode, &arg.intarg, sizeof(arg.intarg));\
            } break;\
        case ARG_TYPE_REG: {\
            dynarr_append(&ass->bytecode, (uint8_t []) { toexecute.opcode | 0b01000000 }, 1);\
            dynarr_append(&ass->bytecode, &arg.regarg.opcode, 1);\
            } break;\
        case ARG_TYPE_NONE: {\
            PRETTY_ERROR("assembler", NOLOGMETA, "argument parse error: %s", ass->tokens[--(ass->tc)]);\
            return 1;\
            } break;\
        default:\
            break;\
    }\
} else {\
    dynarr_append(&ass->bytecode, (uint8_t []) { toexecute.opcode }, 1);\
}\

#define GENERATE_COMMAND(NAME, OPCODE, ARGN, ...)\
    case COMMANDS_##NAME:\
        WRITE_CMD(ARGN)\
        break;\

int assembler_translate(assembler *ass, const char *output) {
    assert(ass->text);
    assert(ass->textsize != 0);
    assert(ass->tokens);
    assert(ass->numtok != 0);
    assert(ass->tc == 0);
    // TODO: validate dynarr
    assert(ass->bytecode.arr);

    PRETTY_LOG("assembler", NOLOGMETA, "Running text translator...");

    while (ass->tc < ass->numtok) {
        char *currtok = ass->tokens[ass->tc++];

        command_t toexecute = _parse_cmd(currtok);

        switch (toexecute.cmd_code) {
            case COMMANDS_NONE: {
                PRETTY_ERROR("assembler", NOLOGMETA, "command parse error: %s", ass->tokens[--(ass->tc)]);\
                return 1;
            } break;

            #include "is.dsl"

            default:
              break;
        }
    }

    int fd = open(output, O_WRONLY|O_CREAT, 0644);
    if (fd < 0) {
        PRETTY_ERROR("assembler", LOGMETA, "open()");
        close(fd);
        return 1;
    }
    
    ssize_t ret = write(fd, SIG, sizeof(SIG));
    if (ret < 0) {
        PRETTY_ERROR("assembler", LOGMETA, "write()");
        close(fd);
        return 1;
    }

    ret = write(fd, ass->bytecode.arr, dynarr_size(&ass->bytecode));
    if (ret < 0) {
        PRETTY_ERROR("assembler", LOGMETA, "write()");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}

#undef GENERATE_COMMAND


int main(int argc, char *argv[]) {
    if (argc != 3) {
        PRETTY_ERROR("assembler", NOLOGMETA, "Usage: %s [SOURCE] [OUTPUT]", *argv);
        PRETTY_FAIL("assembler", "Invalid agruments");
    }

    if ( finode(argv[1]) == finode(argv[2]) ) {
        PRETTY_FAIL("assembler", "input file is the same as output file");
    }

    assembler ass = assembler_init();

    int ret = assembler_loadtext(&ass, /*source file*/argv[1]);

    if (ret) {
        PRETTY_LOG("assembler", NOLOGMETA, RED("text loading error"));
    }

    ret = assembler_translate(&ass, /*output file*/argv[2]);

    if (ret) {
        PRETTY_LOG("assembler", NOLOGMETA, RED("translator error"));
        assembler_dump(&ass);
    }

    assembler_destroy(&ass);

    return 0;
}


