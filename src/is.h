/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef INSTRCTN_SET_H
#define INSTRCTN_SET_H

#include <stdlib.h>
#define SIZEOFARR(arr) sizeof((arr))/sizeof((arr[0]))

#define GENERATE_COMMAND(txt, opcode) COMMANDS_##txt,
typedef enum {
    #include "is.dsl"
} COMMANDS;
#undef GENERATE_COMMAND


typedef struct {
    const char *name;
    const char opcode;
    const size_t len;
    const COMMANDS code;
} command_t;


#define GENERATE_COMMAND(txt, opcode) \
(command_t) {                         \
    .name = #txt,                     \
    .len  = SIZEOFARR(#txt) - 1,      \
    .code = COMMANDS_##txt,           \
},                                    \
 
const command_t INSTRCTN_SET[] = {
    #include "is.dsl"
};

#undef GENERATE_COMMAND

const command_t COMMAND_UNPARSED = {
    .name   = "unparsed",
    .opcode = 0x0,
    .len    = (size_t)-1,
    .code   = COMMANDS_NONE,
};

#endif // INSTRCTN_SET_H
