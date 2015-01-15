#define DEBUG

extern "C" {
#include <stdarg.h>
#include <stdio.h>
#include "../main/debug.h"
}

#include "test.h"
#include "TestBase.h"

static void delim()
{
    debugn("=");

    for (int i = 0; i < 80; i++)
        debugc("=");

    debugnl();
}
int TestBase::run(int num, ...)
{
    debug("Running Tests");
    this->beforeClass();

    va_list ap;
    bool(*f)();
    va_start(ap, num);

    testResult = true;

    int test_succeeded = 0;
    int test_failed = 0;

    for (int i = 0; i < num; i++) {
        this->setup();

        // call test
        f = va_arg(ap, bool(*)());
        int retval = f();
        testResult &= retval;

        if (retval == true) {
            debug("TEST OK");
            test_succeeded++;
        } else {
            debug("TEST FAIL");
            test_failed++;
        }

        this->tearDown();
    }

    va_end(ap);
    this->afterClass();

    delim();
    debug("TEST SUMMARY");
    delim();
    debug("all tests: %d, succeeded: %d, failed: %d",
          test_succeeded + test_failed, test_succeeded, test_failed);
    delim();

    return testResult;
}

void TestBase::setup()
{
}

void TestBase::tearDown()
{
}

void TestBase::afterClass()
{
}

void TestBase::beforeClass()
{
}
