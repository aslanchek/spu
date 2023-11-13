GENERATE_COMMAND(NONE, 0x0, {
    if ( spu->text[spu->cc] == ';' ) {
        PRETTY_LOG(NOLOGMETA, "; encountered. Skipping...");
        // skipping commentaries
        while ( spu->text[spu->cc++] != '\n' );
    } else {
        spu->cc++;
    }
    continue;
})

GENERATE_COMMAND(HLT,  0x1, {
    PRETTY_LOG(NOLOGMETA, "Halting...");
    break;
})

GENERATE_COMMAND(PUSH, 0x2, {
    char *argstr = commandstr + toexecute.len;

    // push argument validation
    if ( *argstr != ' ') {
        PRETTY_LOG(NOLOGMETA, "\"%s\" argument parse error: \"%.*s\"",
            toexecute.name,
             strchr(commandstr, '\n') - commandstr, commandstr);
        return -1;
    }
    int argparsed = atoi( argstr );
    stack_int_push(&spu->stack, argparsed IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(POP,  0x3, {
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(IN,  0x4, {
    int tmp = 0;
    scanf("%d", &tmp);
    stack_int_push(&spu->stack, tmp IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(OUT,   0x5, {
    int top = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    fprintf(stdout, "%d\n", top);
})

GENERATE_COMMAND(ADD,  0x6, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 + op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(SUB,  0x7, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 - op1 IF_VERBOSE(, LOGMETA));
})

GENERATE_COMMAND(MUL,  0x8, {
    int op1 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));
    int op2 = stack_int_top(&spu->stack IF_VERBOSE(, LOGMETA));
    stack_int_pop(&spu->stack IF_VERBOSE(, LOGMETA));

    stack_int_push(&spu->stack, op2 * op1 IF_VERBOSE(, LOGMETA));
})
