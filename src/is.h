/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef INSTRCTN_SET_H
#define INSTRCTN_SET_H

#include <stdlib.h>
#include <stdint.h>

#define SIZEOFARR(arr) sizeof((arr))/sizeof((arr[0]))

#define GENERATE_COMMAND(txt, opcode, text) COMMANDS_##txt,
typedef enum {
    COMMANDS_NONE,

    #include "is.dsl"

} COMMANDS;
#undef GENERATE_COMMAND

typedef struct {
    const char *name;
    const uint8_t opcode;
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
 
static const command_t INSTRCTN_SET[] = {
    #include "is.dsl"
};

#undef GENERATE_COMMAND

static const command_t COMMAND_NONE = (const command_t) {
  .name   = "NONE",
  .opcode = 0,
  .len    = 0,
  .code   = COMMANDS_NONE,
};

#endif // INSTRCTN_SET_H

