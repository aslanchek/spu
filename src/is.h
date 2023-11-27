/*********************************
 * Instruction set description
 *
 *********************************/
#ifndef INSTRCTN_SET_H
#define INSTRCTN_SET_H

#include <stdlib.h>
#include <stdint.h>

#include "dynarr_int.h"

#define SIZEOFARR(arr) sizeof((arr))/sizeof((arr[0]))

#define GENERATE_COMMAND(NAME, OPCODE, ARGN, CODE) COMMANDS_##NAME,
typedef enum {

    COMMANDS_NONE,
    COMMANDS_LABEL,

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

static const command_t COMMAND_LABEL = (const command_t) {
  .name     = "LABEL",
  .opcode   = 0,
  .len      = 0,
  .cmd_code = COMMANDS_LABEL,
};

typedef enum {
    ARG_TYPE_INT,
    ARG_TYPE_REG,
    ARG_TYPE_MEM,
    ARG_TYPE_LAB,
    ARG_TYPE_NONE
} ARG_TYPE;

/*
 * 8 бит
 *  001 00000
 *    ^    ^
 *    imm   \ номер команды
 *
 *  010 00000
 *   ^ 
 *   reg
 *
 *  100 00000
 *  ^ 
 *  mem:
 *  
 *  101 00000
 *    ^
 *    [rax+imm]
 *
 *  110 00000
 *   ^
 *   [rax-imm]
 *
 */
#define COMMANDMASK  0b00011111
#define ARGUMENTMASK 0b11100000

#define ISLAB        0b00000000 // lab - label
#define ISIMM        0b00100000 // imm - immediate constant
#define ISREG        0b01000000
#define ISMEM        0b10000000 // [rax]
#define ISMEM_ADD    0b10100000 // [rax+imm]
#define ISMEM_SUB    0b11000000 // [rax-imm]

#define FXPVAL -1

typedef struct {
    char      *name;
    int        addr;    // address of label
    dynarr_int fxplocs; // fixup locations - where to fill addr of label
} label_t;


typedef struct {
    const char *name;
    const uint8_t opcode;
    const size_t len;
} reg_t;


typedef struct {
    const ARG_TYPE type;
    union {
      reg_t   regarg;
      int     intarg;
      off_t   moffarg;
      label_t labarg;
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

