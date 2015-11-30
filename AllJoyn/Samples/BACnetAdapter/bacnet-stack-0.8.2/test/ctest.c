/* ctest.c: Implements the CTest Framework */

#include "ctest.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* Number of tests to hold incrementally */
enum { CHUNK = 10 };

Test *ct_create(
    const char *name,
    void (*init) (Test *))
{
    int backOutLevel = 0;
    Test *pTest = malloc(sizeof(Test));
    if (pTest) {
        pTest->nPass = pTest->nFail = pTest->nTests = 0;
        pTest->pStream = stdout;

        /* Allocate array of fptrs: */
        assert(CHUNK);
        pTest->pTestFuns = calloc(CHUNK, sizeof(TestFunc));
        if (pTest->pTestFuns) {
            pTest->maxTests = CHUNK;
            /* Allocate test name: */
            assert(name);
            pTest->name = malloc(strlen(name) + 1);
            if (pTest->name)
                strcpy(pTest->name, name);
            else
                ++backOutLevel;
        } else
            ++backOutLevel;
    }

    /* Back-out allocations if memory failed: */
    if (backOutLevel) {
        switch (backOutLevel) {
            case 2:
                free(pTest->pTestFuns);
                pTest->pTestFuns = NULL;
            case 1:
                free(pTest);
                pTest = NULL;
        }
    } else if (init) {
        assert(pTest);
        init(pTest);
    }
    return pTest;
}

void ct_destroy(
    Test * pTest)
{
    assert(pTest);
    assert(pTest->pTestFuns);
    free(pTest->pTestFuns);
    pTest->pTestFuns = NULL;
    assert(pTest->name);
    free(pTest->name);
    pTest->name = NULL;
    free(pTest);
}

bool ct_addTestFunction(
    Test * pTest,
    TestFunc tfun)
{
    assert(pTest);
    assert(pTest->pTestFuns);
    if (pTest->nTests == pTest->maxTests) {
        size_t newSize = pTest->nTests + CHUNK;
        TestFunc *new_pTestFuns = realloc(pTest->pTestFuns,
            newSize * sizeof(TestFunc));
        if (!new_pTestFuns)
            return false;
        pTest->pTestFuns = new_pTestFuns;
        pTest->maxTests += CHUNK;
    }
    assert(pTest->nTests < pTest->maxTests);
    pTest->pTestFuns[pTest->nTests++] = tfun;
    return true;
}

void ct_setStream(
    Test * pTest,
    FILE * pStream)
{
    pTest->pStream = pStream;
}

FILE *ct_getStream(
    Test * pTest)
{
    return pTest->pStream;
}

long ct_report(
    Test * pTest)
{
    assert(pTest);
    if (pTest->pStream) {
        fprintf(pTest->pStream, "Test \"%s\":\n\tPassed: %ld\n\tFailed: %ld\n",
            pTest->name, pTest->nPass, pTest->nFail);
    }
    return pTest->nFail;
}


void ct_succeed(
    Test * pTest)
{
    assert(pTest);
    ++pTest->nPass;
}

void ct_do_test(
    Test * pTest,
    const char *str,
    bool cond,
    const char *file,
    long line)
{
    assert(pTest);
    if (!cond)
        ct_do_fail(pTest, str, file, line);
    else
        ct_succeed(pTest);
}

void ct_do_fail(
    Test * pTest,
    const char *str,
    const char *file,
    long line)
{
    assert(pTest);
    ++pTest->nFail;
    if (pTest->pStream) {
        fprintf(pTest->pStream, "%s failure: (%s), %s (line %ld)\n",
            pTest->name, str, file, line);
    }
}

long ct_getNumPassed(
    Test * pTest)
{
    assert(pTest);
    return pTest->nPass;
}

long ct_getNumFailed(
    Test * pTest)
{
    assert(pTest);
    return pTest->nFail;
}

long ct_run(
    Test * pTest)
{
    size_t testNum;
    assert(pTest);
    for (testNum = 0; testNum < pTest->nTests; ++testNum)
        pTest->pTestFuns[testNum] (pTest);
    return pTest->nFail;
}

void ct_reset(
    Test * pTest)
{
    assert(pTest);
    pTest->nFail = pTest->nPass = 0;
}

const char *ct_getName(
    Test * pTest)
{
    assert(pTest);
    return (pTest->name);
}

long ct_getNumTests(
    Test * pTest)
{
    assert(pTest);
    return pTest->nTests;
}
