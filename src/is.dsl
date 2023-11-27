GENERATE_COMMAND(HLT,  0b11111, 0, {
    PRETTY_LOG("spu", NOLOGMETA, "Halting...");
    //goto halt;
    return 0;
})

GENERATE_COMMAND(PUSH, 0b00001, 1, {
    GETARG(arg);
    stack_int_push(&spu->stack, arg IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(POP,  0b00010, 1, {
    GETREG(reg);
    *reg = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(IN,   0b00011, 0, {
    int tmp = 0;
    scanf("%d", &tmp);
    stack_int_push(&spu->stack, tmp IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(OUT,  0b00100, 0, {
    int top = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    fprintf(stdout, "%d\n", top);
})

GENERATE_COMMAND(ADD,  0b00101, 0, {
    POPSTACK(op1)
    POPSTACK(op2)

    stack_int_push(&spu->stack, op2 + op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(SUB,  0b000110, 0, {
    POPSTACK(op1)
    POPSTACK(op2)

    stack_int_push(&spu->stack, op2 - op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(MUL, 0b000111, 0, {
    POPSTACK(op1)
    POPSTACK(op2)

    stack_int_push(&spu->stack, op2 * op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(JA,  0b001000, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 > op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JAE, 0b001001, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 >= op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JB,  0b001010, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 < op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JBE, 0b001011, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 < op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JE,  0b001100, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 == op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JNE, 0b001101, 1, {
    GETARG(arg)

    POPSTACK(op1)
    POPSTACK(op2)

    if (op2 != op1) {
        spu->pc = arg;
    }
})

GENERATE_COMMAND(JMP, 0b001110, 1, {
    GETARG(arg)

    spu->pc = arg;
})

GENERATE_COMMAND(JM,  0b001111, 1, {
    GETARG(arg)

    spu->pc = arg;
})

