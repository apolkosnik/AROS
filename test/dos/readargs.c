/*
 * Copyright (C) 2011, The AROS Development Team.  All rights reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the AROS PUBLIC LICENSE (APL) Version 1.1
 */

#include <string.h>     /* for memset() */

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <dos/stdio.h>
#include <exec/lists.h>

/* For compatibility with AOS, our test reference */
static AROS_UFH2(VOID, _SPutC,
        AROS_UFHA(BYTE, c, D0),
        AROS_UFHA(BYTE **, ptr, A3))
{
    AROS_USERFUNC_INIT

    **ptr = c;

    (*ptr)++;

    AROS_USERFUNC_EXIT
}

VOID SPrintf( char *target, const char *format, ...)
{
    RawDoFmt( format, (APTR)&(((ULONG *)&format)[1]), _SPutC, &target);
}
    
#define TEST_START(name) do { \
    CONST_STRPTR test_name = name ; \
    struct List expr_list; \
    int failed = 0; \
    tests++; \
    NEWLIST(&expr_list);

#define VERIFY_EQ(retval, expected) \
    do { \
        __typeof__(retval) val = retval; \
        if (val != (expected)) { \
            static char buff[128]; \
            static struct Node expr_node; \
            SPrintf(buff, "%s (%ld) != %ld", #retval , (LONG)val, (LONG)expected); \
            expr_node.ln_Name = buff; \
            AddTail(&expr_list, &expr_node); \
            failed |= 1; \
        } \
    } while (0)

#define VERIFY_STREQ(retval, expected) \
    do { \
        CONST_STRPTR val = (CONST_STRPTR)(retval); \
        if (strcmp(val,(expected)) != 0) { \
            static char buff[128]; \
            static struct Node expr_node; \
            SPrintf(buff, "%s (%s) != %s", #retval, val, (LONG)expected); \
            expr_node.ln_Name = buff; \
            AddTail(&expr_list, &expr_node); \
            failed |= 1; \
        } \
    } while (0)


#define VERIFY_RET(func, ret, reterr) \
    do { \
        SetIoErr(-1); \
        VERIFY_EQ(func, ret); \
        VERIFY_EQ(IoErr(), reterr); \
    } while (0)

#define TEST_END() \
    if (failed) { \
        struct Node *node; \
        tests_failed++; \
        Printf("Test %ld: Failed (%s)\n", (LONG)tests, test_name); \
        ForeachNode(&expr_list, node) { \
            Printf("\t%s\n", node->ln_Name); \
        } \
    } \
} while (0)

static inline SIPTR readargs_file(CONST_STRPTR format, IPTR *args, CONST_STRPTR input, struct RDArgs **retp)
{
    SIPTR retval;
    struct RDArgs *ret;
    BPTR oldin;
    BPTR io;
    CONST_STRPTR tmpfile = "RAM:readargs.test";
    BPTR oldout;
    oldout = SelectOutput(Open("NIL:", MODE_NEWFILE));
   
    io = Open(tmpfile, MODE_NEWFILE);
    Write(io, input, strlen(input));
    Close(io);

    io = Open(tmpfile, MODE_OLDFILE);

    oldin = Input();
    SelectInput(io);
    SetIoErr(0);
    ret = ReadArgs(format, args, NULL);

    *retp = ret;
    retval = (ret != NULL) ? RETURN_OK : IoErr();

    Close(SelectInput(oldin));
    Close(SelectOutput(oldout));

    DeleteFile(tmpfile);

    return retval;
}

#define TEST_READARGS(format, input) \
    TEST_START(format " '" input "'"); \
    CONST_STRPTR args[10] = { "inv1", "inv2", "inv3" }; \
    SIPTR ioerr; \
    struct RDArgs *ret = NULL; \
    ioerr = readargs_file(format, (IPTR *)&args[0], input, &ret);

#define TEST_ENDARGS() \
        if (ret) FreeArgs(ret); \
        TEST_END();

int main(int argc, char **argv)
{
    int tests = 0, tests_failed = 0;

    TEST_READARGS("KEYA","val1\n");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","val1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","?\nval1\n");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","?\nval1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","keya val1\n");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","keya val1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA","keya=val1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","val1 val2");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","keya=val1 keyb=val2");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","keya val1 keyb val2");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","keya val1 val2");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","keyb val2 val1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB","keyb val2 keya val1");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB/F","val1 val2 val3");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2 val3");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB/F","val1 val2 val3=val4  \"val5\" keya=val6");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2 val3=val4  \"val5\" keya=val6");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB/F","val1 val2 val3\n");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2 val3");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB/F,KEYc","val1 keyc val4 val2 val3\n");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(args[1], "val2 val3");
        VERIFY_STREQ(args[2], "val4");
    TEST_ENDARGS();

    TEST_READARGS("KEYA,KEYB/M","val1 val2 val3");
        VERIFY_EQ(ioerr, RETURN_OK);
        VERIFY_STREQ(args[0], "val1");
        VERIFY_STREQ(((CONST_STRPTR *)args[1])[0], "val2");
        VERIFY_STREQ(((CONST_STRPTR *)args[1])[1], "val3");
        VERIFY_EQ(((CONST_STRPTR *)args[1])[2], NULL);
    TEST_ENDARGS();

    if (tests_failed == 0)
        Printf("All %ld test passed\n", (LONG)tests);
    else
        Printf("%ld of %ld tests failed\n", (LONG)tests_failed, (LONG)tests);

    Flush(Output());

    return 0;
}
