#include "asm.h"
#include "dynarr_byte.h"
#include "dynarr_int.h"
#include "is.h"
#include "log.h"

assembler assembler_init() {
    assembler new = {
        .text        = NULL,
        .textsize    = 0,
        .tokens      = NULL,
        .numtok      = 0,
        .tc          = 0,
        .bytecode    = dynarr_byte_init(1),
        .labels      = NULL,
        .numlabels   = 0,
    };

    return new;
}

void assembler_destroy(assembler *ass) {
    assert(ass->text);
    assert(ass->tokens);
    assert(ass->bytecode.arr);
    assert(ass->tc == ass->numtok);
    assert(ass->labels);

    free(ass->text);
    free(ass->tokens);

    for (size_t i = 0; i < ass->numlabels; i++) {
        dynarr_int_destroy(&ass->labels[i].fxplocs);
    }
    free(ass->labels);

    dynarr_byte_destroy(&ass->bytecode);

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


    fprintf(stderr, "+--- LABELS ---\n");

    for (size_t i = 0; i < ass->numlabels; i++) {
        fprintf(stderr, "l%zu: \"%s\" [%03d]: ", i, ass->labels[i].name, ass->labels[i].addr);
        for (size_t j = 0; j < ass->labels[i].fxplocs.size; j++) {
            fprintf(stderr, "[%03d] ", dynarr_int_accss(&ass->labels[i].fxplocs, j) );
        }
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");

    fprintf(stderr, "+----------------\n");



    fprintf(stderr, "+--- BYTECODE ---\n");

    for (size_t i = 0; i < dynarr_byte_size(&ass->bytecode); i++) {
        fprintf(stderr, "[%03zu] 0x%02x          ""0b"
                BYTE2BIN_PATTERN " \n", i,
                dynarr_byte_accss(&ass->bytecode, i),
                BYTE2BIN(dynarr_byte_accss(&ass->bytecode, i)));
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

    PRETTY_LOG("assembler", NOLOGMETA, "Running loader...");

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

    // extracting tokens

    char delims[] = " \n\t";

    size_t nt = 0;
    char *token = strtok(ass->text, delims);
    while (token) {
        ass->tokens[nt++] = token;
        token = strtok(NULL, delims);
    }

    // getting labels number
    size_t numlabels = 0;

    for (size_t i = 0; i < ass->numtok; i++) {
        if (strchr(ass->tokens[i], ':')) {
            numlabels++;
        }
    }

    ass->labels = calloc(numlabels, sizeof(label_t));

    // extracting labels
    for (size_t i = 0; i < ass->numtok; i++) {
        char *colon = NULL;
        if ( ( colon = strchr(ass->tokens[i], ':') ) == ( ass->tokens[i] + strlen(ass->tokens[i]) - 1 ) ) {
            ass->labels[ass->numlabels++] = (label_t) { .name    = ass->tokens[i],
                                                        .addr    = FXPVAL,
                                                        .fxplocs = dynarr_int_init(1) };
        }
    }

    return 0;
}


command_t _parse_cmd(char *token) {
    // check if label with : encountered
    // "l1:"
    char *colon = NULL;
    if ( ( colon = strchr(token, ':') ) == ( token + strlen(token) - 1 ) ) {
        return COMMAND_LABEL;
    }


    for(size_t i = 0; i < SIZEOFARR(INSTRCTN_SET); i++) {
        if(strncasecmp(INSTRCTN_SET[i].name, token, strlen(INSTRCTN_SET[i].name)) == 0) {
            PRETTY_LOG("assembler translate", NOLOGMETA, "\"%s\" encountered", INSTRCTN_SET[i].name);
            return INSTRCTN_SET[i];
        }
    }

    return /*failed to parse*/COMMAND_NONE;
}

/*
 *
    // check if it is memory access
    if ( token[0] == '[' && token[strlen(token)] == ']' ) {
        // allowed: [reg] or [reg+-imm]
        //      [rax]
        //      [rax+7]
        //      [rax-5]
        //
        // restricted:
        //      [rax+5.5]
        //      [8+rax]
        //      [-18+rax]
        //      [-rax]
        //      [+rax]
        PRETTY_LOG("assembler", NOLOGMETA, "memory access");

        char *op = NULL;

        op = strchr(token, '+');
        if (op) {

        }

        op = strchr(token, '-');
        if (op) {

        }

        if (NULL) {

        }

    }

 *
 */

arg_t _parse_arg(char *token, assembler *ass) {
    // check if arg is label
    for (size_t i = 0; i < ass->numlabels; i++) {
        if (strncasecmp(ass->labels[i].name, token, strlen(token)) == 0) {
            PRETTY_LOG("assembler translate", NOLOGMETA, "label argument encountered: %s", ass->labels[i].name);
            dynarr_int_append(&ass->labels[i].fxplocs, ass->bytecode.size + 1);
            return (const arg_t) { .type = ARG_TYPE_LAB, .intarg = FXPVAL };
        }
    }

    // check if arg is register
    for(size_t i = 0; i < SIZEOFARR(REGS_SET); i++) {
        if (strncasecmp(REGS_SET[i].name, token, strlen(REGS_SET[i].name)) == 0) {
            PRETTY_LOG("assembler translate", NOLOGMETA, "\"%s\" encountered", REGS_SET[i].name);
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

/* TODO: релизовать следующую фичу:
         в is.dsl добавить параметр, отвечающий за
         типы аргументов, которые может принимать инструкция,
         например push может принимать imm и reg (0b11) , а вот
         pop только reg (0b01), а вот add ничего не принимает (0b00).
         Научиться корректно это обрабатывать, если b00, то
         вообще не генерирует код читающий аргументы, если 
         b01 - генерировать только тот код, который читаем imm, и тп.

         Это позволит обрабатывать случай, когда аргументом pop является imm.
*/

#define WRITE_CMD(ARGN)\
if (ARGN) {\
    const arg_t arg = _parse_arg(ass->tokens[ass->tc++], ass);\
    switch ( arg.type ) {\
        case ARG_TYPE_INT: {\
            dynarr_byte_append(&ass->bytecode, (uint8_t []) { toexecute.opcode | ISIMM }, 1);\
            dynarr_byte_append(&ass->bytecode, &arg.intarg, sizeof(arg.intarg));\
            } break;\
        case ARG_TYPE_REG: {\
            dynarr_byte_append(&ass->bytecode, (uint8_t []) { toexecute.opcode | ISREG }, 1);\
            dynarr_byte_append(&ass->bytecode, &arg.regarg.opcode, 1);\
            } break;\
        case ARG_TYPE_LAB: {\
            dynarr_byte_append(&ass->bytecode, (uint8_t []) { toexecute.opcode | ISLAB }, 1);\
            dynarr_byte_append(&ass->bytecode, &arg.intarg, sizeof(arg.intarg));\
            } break;\
        case ARG_TYPE_NONE: {\
            PRETTY_ERROR("assembler translate", NOLOGMETA, "argument parse error: %s", ass->tokens[--(ass->tc)]);\
            return 1;\
            } break;\
        default:\
            break;\
    }\
} else {\
    dynarr_byte_append(&ass->bytecode, (uint8_t []) { toexecute.opcode }, 1);\
}\

#define GENERATE_COMMAND(NAME, OPCODE, ARGN, ...)\
    case COMMANDS_##NAME:\
        WRITE_CMD(ARGN)\
        break;\

int assembler_translate(assembler *ass) {
    assert(ass->text);
    assert(ass->textsize != 0);
    assert(ass->tokens);
    assert(ass->numtok != 0);
    assert(ass->tc == 0);
    assert(ass->labels);
    // TODO: validate dynarr
    assert(ass->bytecode.arr);

    PRETTY_LOG("assembler", NOLOGMETA, "Running text translator...");

    while (ass->tc < ass->numtok) {
        char *currtok = ass->tokens[ass->tc++];

        command_t toexecute = _parse_cmd(currtok);

        switch (toexecute.cmd_code) {
            case COMMANDS_NONE: {
                PRETTY_ERROR("assembler translate", NOLOGMETA, "command parse error: %s", ass->tokens[--(ass->tc)]);
                return 1;
            } break;

            case COMMANDS_LABEL: {
                PRETTY_LOG("assembler translate", NOLOGMETA, "label encountered: ", currtok);

                for (size_t i = 0; i < ass->numlabels; i++) {
                    if (strncasecmp(currtok, ass->labels[i].name, strlen(ass->labels[i].name)) == 0) {
                        if (ass->labels[i].addr != FXPVAL) {
                            PRETTY_ERROR("assembler translate", NOLOGMETA, "label redefinition: %s", ass->tokens[--(ass->tc)]);
                            return 1;
                        }

                        ass->labels[i].addr = ass->bytecode.size;
                    }
                }
            } break;

            #include "is.dsl"

            default:
              break;
        }
    }

    return 0;
}

#undef GENERATE_COMMAND

int assembler_link(assembler *ass) {
    assert(ass->text);
    assert(ass->tokens);
    assert(ass->labels);
    // TODO: validate dynarr
    assert(ass->bytecode.arr);

    PRETTY_LOG("assembler", NOLOGMETA, "Running linker...");

    for (size_t i = 0; i < ass->numlabels; i++) {
        for (size_t j = 0; j < ass->labels[i].fxplocs.size; j++) {
            dynarr_byte_insert(&ass->bytecode, &ass->labels[i].addr, sizeof(int), dynarr_int_accss(&ass->labels[i].fxplocs, j) );
        }
    }

    for (size_t i = 0; i < ass->bytecode.size; i++) {
        uint8_t toint[sizeof(int)];
        memcpy(toint,  ass->bytecode.arr + i, sizeof(int));

        int faultlabel = *(int *)(toint);

        if (faultlabel == FXPVAL) {
            PRETTY_ERROR("assembler link", NOLOGMETA, "unresolved label: on cs:+%zu", i);\
            return 1;
        }
    }

    return 0;
}

int assembler_write(assembler *ass, const char *output) {
    assert(ass->text);
    assert(ass->tokens);
    assert(ass->labels);
    // TODO: validate dynarr
    assert(ass->bytecode.arr);

    PRETTY_LOG("assembler", NOLOGMETA, "Running writer...");

    int fd = open(output, O_WRONLY|O_CREAT, 0644);
    if (fd < 0) {
        PRETTY_ERROR("assembler write", LOGMETA, "open()");
        close(fd);
        return 1;
    }
    
    ssize_t ret = write(fd, SIG, sizeof(SIG));
    if (ret < 0) {
        PRETTY_ERROR("assembler write", LOGMETA, "write()");
        close(fd);
        return 1;
    }

    ret = write(fd, ass->bytecode.arr, dynarr_byte_size(&ass->bytecode));
    if (ret < 0) {
        PRETTY_ERROR("assembler write", LOGMETA, "write()");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}



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
        PRETTY_ERROR("assembler", NOLOGMETA, "text loading error");
        assembler_destroy(&ass);
        return 1;
    }

    ret = assembler_translate(&ass);

    if (ret) {
        PRETTY_ERROR("assembler", NOLOGMETA, "translator error");
        assembler_dump(&ass);
        assembler_destroy(&ass);
        return 1;
    }

    ret = assembler_link(&ass);

    if (ret) {
        PRETTY_ERROR("assembler", NOLOGMETA, "linker error");
        assembler_dump(&ass);
        assembler_destroy(&ass);
        return 1;
    }

    ret = assembler_write(&ass, /*output file*/argv[2]);

    if (ret) {
        PRETTY_ERROR("assembler", NOLOGMETA, "writer error");
        #ifndef DEBUG
        assembler_dump(&ass);
        #endif
        assembler_destroy(&ass);
        return 1;
    }

    #ifdef DEBUG
    assembler_dump(&ass);
    #endif

    assembler_destroy(&ass);

    return 0;
}


