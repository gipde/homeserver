#define DEBUG

extern "C" {
#include <stdarg.h>
#include <stdio.h>
#include "../../main/debug.h"
}

#include "test.h"
#include "TestBase.h"

int TestBase::run(int num, ...)
{
    debug("Running Tests");
    this->beforeClass();

    va_list ap;
    bool(*f)();
    va_start(ap, num);

    testResult = true;

    for (int i = 0; i < num; i++) {
        this->setup();

        // call test
        f = va_arg(ap, bool(*)());
        int retval = f();
        testResult &= retval;
        debug("Retval: %d State: %d", retval, testResult);

        if (retval) {
            debug("OK");
        } else {
            debug("FAIL");
        }

        this->tearDown();
    }

    va_end(ap);
    this->afterClass();
    return testResult;
}

void TestBase::setup()
{
    debug("Base Setup");
}

void TestBase::tearDown()
{
    debug("Base Teardown");
}

void TestBase::afterClass()
{
    debug("After Class");
}

void TestBase::beforeClass()
{
    debug("Before Class");
}
