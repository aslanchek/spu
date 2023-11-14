#include "asm.h"
#include "is.h"
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
    //assert(ass->bytecode);
    assert(ass->tc == ass->numtok);

    free(ass->text);
    free(ass->tokens);
    //free(ass->bytecode);

    bzero(ass, sizeof(assembler));
}


void assembler_dump(assembler *ass) {
    fprintf(stderr, "+-------TOKENS------\n");


    for (size_t i = 0; i < ass->numtok; i++) {
        fprintf(stderr, "[%05zu] %s\n", i, ass->tokens[i]);
    }

    fprintf(stderr, "+-------------------\n"
                    "total tokens number = %zu\n", ass->numtok);
}

int assembler_loadtext(assembler *spu, char *source) {
    // reads programm text from filename
    int fd = open(source, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("assembler", "open()");
    }

    ssize_t size = fsize(fd);
    if (size < 0) {
        PRETTY_ERROR("assembler", "Faulty reading file stat");
    }

    spu->textsize = (size_t)size;
    
    spu->text = calloc(spu->textsize + 1, 1);

    read(fd, spu->text, spu->textsize);

    close(fd);

    for (char *ptr = spu->text; ptr < spu->text + spu->textsize; ptr++) {
        if (*ptr == ';') while (*ptr != '\n') { *ptr++= ' '; }
    }
    
    for (size_t i = 0; i < spu->textsize; i++) {
        if (!isspace(spu->text[i])) {
            spu->numtok++;
            while (!isspace(spu->text[++i]));
        }
    }

    spu->tokens = calloc(spu->numtok, sizeof(char *));

    char delims[] = " \n\t";

    size_t nt = 0;
    char *token = strtok(spu->text, delims);
    while (token) {
        spu->tokens[nt++] = token;
        token = strtok(NULL, delims);
    }

    return 0;
}


/*
 * parses arbitrary string:
 *
 *  [push 123\n]      -> push
 *  [pop\n]           -> pop
 *  [addafadsfadsf\n] -> add
 *  [3.14push\n]      -> none
 *
 */
command_t _parse_cmd(char *strline) {
    for(size_t i = 0; i < SIZEOFARR(INSTRCTN_SET); i++) {
        if(strncasecmp(INSTRCTN_SET[i].name, strline, strlen(INSTRCTN_SET[i].name)) == 0) {
            PRETTY_LOG("assembler", NOLOGMETA, "\"%s\" encountered", INSTRCTN_SET[i].name);
            return INSTRCTN_SET[i];
        }
    }

    return /*failed to parse*/COMMAND_NONE;
}

/*
 * text = [ push 5\npush rax\nadd\nhlt ] -> [ "push", "5", "push", "rax", "add", "hlt" ]
 *
 *
 *
 */


#define GENERATE_COMMAND(TXT, OPCODE, ...)\
    case COMMANDS_##TXT:\
        fprintf(stderr, "parsed token: \"%s\" -> %s\n", currtok, toexecute.name);\
        break;\

int assembler_translate(assembler *ass) {
    assert(ass->text);
    assert(ass->textsize != 0);
    assert(ass->tc == 0);

    assembler_dump(ass);
    
    PRETTY_LOG("assembler", NOLOGMETA, "Running text translator...");

    while (ass->tc < ass->numtok) {
        char *currtok = ass->tokens[ass->tc];
        command_t toexecute = _parse_cmd(currtok);

        switch (toexecute.cmd_code) {
            #include "is.dsl"

            default:
                break;
        }

        ass->tc++;
    }

    return 0;
}

#undef GENERATE_COMMAND


int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRETTY_LOG("assembler", NOLOGMETA, "Usage: %s [SOURCE] [OUTPUT]", *argv);
        PRETTY_FAIL("assembler", "Invalid agruments");
    }
    assembler ass = assembler_init();

    assembler_loadtext(&ass, /*source file*/argv[1]);

    int ret = assembler_translate(&ass);
    if (ret) {
        PRETTY_LOG("assembler;", NOLOGMETA, RED("interpeter error"));
        assembler_dump(&ass);
    }

    assembler_destroy(&ass);
    return 0;
}

