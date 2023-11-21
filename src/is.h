/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef INSTRCTN_SET_H
#define INSTRCTN_SET_H

#include <stdlib.h>
#include <stdint.h>

#define SIZEOFARR(arr) sizeof((arr))/sizeof((arr[0]))

#define GENERATE_COMMAND(NAME, OPCODE, ARGN, CODE) COMMANDS_##NAME,
typedef enum {

    COMMANDS_NONE,

    #include "is.dsl"

} COMMANDS;
#undef GENERATE_COMMAND

typedef struct {
    const char *name;
    const uint8_t opcode;
    const size_t len;
    const COMMANDS cmd_code;
} command_t;


#define GENERATE_COMMAND(NAME, OPCODE, ARGN, CODE) \
(command_t) {                                      \
    .name       = #NAME,                           \
    .opcode     = OPCODE,                          \
    .len        = SIZEOFARR(#NAME) - 1,            \
    .cmd_code   = COMMANDS_##NAME,                 \
},                                                 \

static const command_t INSTRCTN_SET[] = {
    #include "is.dsl"
};
#undef GENERATE_COMMAND

static const command_t COMMAND_NONE = (const command_t) {
  .name     = "NONE",
  .opcode   = 0,
  .len      = 0,
  .cmd_code = COMMANDS_NONE,
};

typedef enum {
    ARG_TYPE_INT,
    ARG_TYPE_REG,
    ARG_TYPE_NONE
} ARG_TYPE;

typedef struct {
    const char *name;
    const uint8_t opcode;
    const size_t len;
} reg_t;

typedef struct {
    const ARG_TYPE type;
    union {
      reg_t regarg;
      int   intarg;
    };
} arg_t;


static const arg_t ARG_NONE = {
    .type = ARG_TYPE_NONE
};

#define GENERATE_REGISTER(NAME, OPCODE)  \
(reg_t) {                                \
    .name     = #NAME,                   \
    .opcode   = OPCODE,                  \
    .len      = SIZEOFARR(#NAME) - 1,    \
},                                       \

static const reg_t REGS_SET[] = {
    #include "registers.dsl"
};

#undef GENERATE_REGISTER

static const reg_t REG_NONE = (const reg_t) {
  .name     = "NONE",
  .opcode   = 0,
  .len      = 0,
};

#endif // INSTRCTN_SET_H

