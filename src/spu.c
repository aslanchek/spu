#include "spu.h"

SPU SPU_init() {
    SPU new = {
       .stack    = stack_int_init(),
       .text     = NULL,
       .textsize = 0,
       .pc       = 0,
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
 *
 *
 *            |       
 *            V       
 *  0x00: -->[41] 01  00  21  1f  00  00  00
 *  0x08:     00  21  39  05  00  00  00  21
 *  0x10:     39  05  00  00  00  41  02  00
 *  0x18:     02  02  02  07  05  41  03  00
 *  0x20:     06  ff
 *  
 *  total text size = 134
 */
void SPU_dump(SPU *spu) {
    STACK_DUMP(&spu->stack, int);

    fprintf(stderr, "\n\n");

    fprintf(stderr, " program counter = %zu\n", spu->pc);

    const size_t linesize = 8;
    
    if ( spu->textsize != 0 ) {
        fprintf(stderr, "            ");
        for (size_t i = 0; i < spu->pc % linesize; i++) {
            fprintf(stderr, "    ");
        }
        fprintf(stderr, " |  \n");

        fprintf(stderr, "            ");
        for (size_t i = 0; i < spu->pc % linesize; i++) {
            fprintf(stderr, "    ");
        }
        fprintf(stderr, " V ");
    }


    for (size_t i = 0; i < spu->textsize; i++) {
        if ( i % linesize == 0 ) {
            if ( i / linesize == spu->pc /linesize ) {
                fprintf(stderr, "\n 0x%04x: -->", (uint8_t) i);
            } else {
                fprintf(stderr, "\n 0x%04x:    ", (uint8_t) i);
            }
        }
        if ( i == spu->pc ) {
            fprintf(stderr, "[%02x]", spu->text[i]);
        } else {
            fprintf(stderr, " %02x ", spu->text[i]);
        }
    }

    fprintf(stderr, "\n total text size = %zu\n", spu->textsize);
}

int SPU_loadtext(SPU *spu, char *filename) {
    // reads programm text from filename
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        PRETTY_ERROR("spu", LOGMETA, "open()");
        return 1;
    }

    ssize_t size = fsize(fd);

    if (size < 0) {
        PRETTY_ERROR("spu", LOGMETA, "Faulty reading file stat");
        close(fd);
        return 1;
    }

    // checking executable signature
    if (size >= 8) {
        uint8_t sig[8];
        read(fd, sig, 8);
        if (*(uint64_t *)sig == 0x0000000000000000U) {
            PRETTY_ERROR("spu", LOGMETA, "Wrong executable signature");
            close(fd);
            return 1;
        }
    } else {
        PRETTY_ERROR("spu", LOGMETA, "Faulty executable signature");
        close(fd);
        return 1;

    }

    spu->textsize = (size_t)size - 8;
    
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


int SPU_run(SPU *spu) {
    assert(!stack_int_validate(&spu->stack));
    assert(spu->text);
    assert(spu->textsize != 0);
    assert(spu->pc == 0);
    
    PRETTY_LOG("spu", NOLOGMETA, "Running text...");

    while (spu->pc < spu->textsize) {
        SPU_dump(spu);
        sleep(1);
        system("clear");
        spu->pc++;
    }

    return 0;
}

#undef GENERATE_COMMAND


int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRETTY_LOG("spu", NOLOGMETA, "Usage: %s [FILE]", *argv);
        PRETTY_FAIL("spu", "Invalid agruments");
    }

    SPU Richard = SPU_init();

    int ret = SPU_loadtext(&Richard, /*input file*/argv[1]);

    if (ret) {
        PRETTY_ERROR("spu", NOLOGMETA, "Loading text error");
        SPU_dump(&Richard);
    }

    ret = SPU_run(&Richard);

    if (ret) {
        PRETTY_ERROR("spu", NOLOGMETA, "Runtime error");
        SPU_dump(&Richard);
    }

    SPU_destroy(&Richard);
    return 0;
}

