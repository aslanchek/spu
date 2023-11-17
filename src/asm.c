#include "asm.h"
#include "is.h"
#include "log.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

assembler assembler_init() {
    assembler new = {
        .text     = NULL,
        .textsize = 0,
        .tokens   = NULL,
        .numtok   = 0,
        .tc       = 0,
        .bytecode = NULL,
    };

    return new;
}

void assembler_destroy(assembler *ass) {
    assert(ass->text);
    assert(ass->tokens);
    assert(ass->bytecode);
    assert(ass->tc == ass->numtok);

    free(ass->text);
    free(ass->tokens);
    free(ass->bytecode);

    bzero(ass, sizeof(assembler));
}


void assembler_dump(assembler *ass) {
    assert(ass->tokens);

    fprintf(stderr, "+---- ASM DUMP -----\n");
    fprintf(stderr, "+--- TOKENS ---\n");


    for (size_t i = 0; i < ass->numtok; i++) {
        fprintf(stderr, "[%05zu] %s\n", i, ass->tokens[i]);
    }

    fprintf(stderr, "+--------------\n"
                    "total tokens number = %zu\n", ass->numtok);

    fprintf(stderr, "--- BYTECODE --\n");

    for (size_t i = 0; i < ass->numtok; i++) {
        fprintf(stderr, "0x%08x ", ass->bytecode[i]);
    }

    fprintf(stderr, "\n");

    for (size_t i = 0; i < ass->numtok; i++) {
        fprintf(stderr, "0b%08b ", ass->bytecode[i]);
    }

    fprintf(stderr, "\n");

    fprintf(stderr, "+--------------\n");

}

/*
 * text = [ push 5\npush rax\nadd\nhlt ] -> [ "push", "5", "push", "rax", "add", "hlt" ]
 */

int assembler_loadtext(assembler *ass, char *source) {
    assert(ass->text     == NULL);
    assert(ass->tokens   == NULL);
    assert(ass->bytecode == NULL);
    assert(ass->textsize != 0);

    //- reads programm text from filename -------------------
    int fd = open(source, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("assembler", "open()");
    }

    ssize_t size = fsize(fd);
    if (size < 0) {
        PRETTY_ERROR("assembler", "Faulty reading file stat");
    }

    ass->textsize = (size_t)size;
    
    ass->text = calloc(ass->textsize + 1, 1);

    read(fd, ass->text, ass->textsize);

    close(fd);

    //- tokenizing ------------------------------------------
    
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

    ass->bytecode = calloc(ass->numtok, sizeof(uint8_t));

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

int arg_cmp(const arg_t a1, const arg_t a2) {
    return (a1.type == a2.type);
}


const arg_t _parse_arg(char *token) {
    for(size_t i = 0; i < SIZEOFARR(REGS_SET); i++) {
        if (strncasecmp(REGS_SET[i].name, token, strlen(REGS_SET[i].name)) == 0) {
            return (arg_t) { .type = ARG_TYPE_REG, .regarg = REGS_SET[i] };
        }
    }

    // тут используем strtod

    return /*failed to parse*/ARG_NONE;
}

int assembler_translate(assembler *ass) {
    assert(ass->text);
    assert(ass->textsize != 0);
    assert(ass->tc == 0);

    assembler_dump(ass);
    
    PRETTY_LOG("assembler", NOLOGMETA, "Running text translator...");

    size_t bcc = 0; //<- bytecode counter

    while (ass->tc < ass->numtok) {
        char *currtok = ass->tokens[ass->tc++];

        command_t toexecute = _parse_cmd(currtok);

        switch (toexecute.cmd_code) {
            case COMMANDS_HLT: {
                ass->bytecode[bcc++] = toexecute.opcode;
                break;

            } break;

            case COMMANDS_PUSH: {
                const arg_t arg = _parse_arg(ass->tokens[ass->tc]);

                switch ( arg.type ) {
                    case ARG_TYPE_INT: {
                        ass->bytecode[bcc++] = toexecute.opcode | 0b00100000;
                        ass->bytecode[bcc++] = arg.intarg;
                        } break;

                    case ARG_TYPE_REG: {
                        ass->bytecode[bcc++] = toexecute.opcode | 0b01000000;
                        ass->bytecode[bcc++] = arg.regarg.opcode;
                        } break;


                    default:
                        PRETTY_LOG("assembler;", NOLOGMETA,
                                   RED("argument parse error"));
                        return 1;
                        break;
                }

            } break;

            case COMMANDS_POP: {
                ass->bytecode[bcc++] = toexecute.opcode;
                break;
            }

            default:
              break;
        }
    }

    assembler_dump(ass);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRETTY_LOG("assembler", NOLOGMETA, "Usage: %s [SOURCE] [OUTPUT]", *argv);
        PRETTY_FAIL("assembler", "Invalid agruments");
    }
    assembler ass = assembler_init();

    assembler_loadtext(&ass, /*source file*/argv[1]);

    int ret = assembler_translate(&ass);
    if (ret) {
        PRETTY_LOG("assembler;", NOLOGMETA, RED("translator error"));
        assembler_dump(&ass);
    }

    assembler_destroy(&ass);
    return 0;
}


