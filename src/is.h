/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef INSTRCTN_SET_H
#define INSTRCTN_SET_H

#include <stdlib.h>
#define SIZEOFARR(arr) sizeof((arr))/sizeof((arr[0]))

#define GENERATE_COMMAND(txt, opcode, text) COMMANDS_##txt,
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


#define GENERATE_COMMAND(TXT, OPCODE, ACTION) \
(command_t) {                                 \
    .name = #TXT,                             \
    .opcode = OPCODE,                         \
    .len  = SIZEOFARR(#TXT) - 1,              \
    .code = COMMANDS_##TXT,                   \
},                                            \
 
const command_t INSTRCTN_SET[] = {
    #include "is.dsl"
};

#undef GENERATE_COMMAND

#endif // INSTRCTN_SET_H

