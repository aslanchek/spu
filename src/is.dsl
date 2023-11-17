GENERATE_COMMAND(HLT,  0b11111111, 0, {
    PRETTY_LOG("spu", NOLOGMETA, "Halting...");
    break;
})

GENERATE_COMMAND(PUSH, 0b00000001, 1, {
    // push argument validation
    if ( *argstr != ' ') {
        PRETTY_LOG("spu", NOLOGMETA, "\"%s\" argument parse error: \"%.*s\"",
            toexecute.name,
             strchr(commandstr, '\n') - commandstr, commandstr);
        return -1;
    }
    int argparsed = atoi( argstr );
    stack_int_push(&spu->stack, argparsed IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(POP,  0b00000010, 0, {
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(IN,   0b00000011, 0, {
    int tmp = 0;
    scanf("%d", &tmp);
    stack_int_push(&spu->stack, tmp IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(OUT,   0b00000100, 0, {
    int top = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    fprintf(stdout, "%d\n", top);
})

GENERATE_COMMAND(ADD,  0b00000101, 0, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 + op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(SUB,  0b00000110, 0, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 - op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(MUL,  0b00000111, 0, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 * op1 IF_VERBOSE(, LOGMETA));
})
