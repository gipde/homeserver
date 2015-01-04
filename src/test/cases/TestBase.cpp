#define DEBUG

extern "C" {
#include <stdarg.h>
#include <stdio.h>
#include "../../main/debug.h"
}

#include "test.h"
#include "TestBase.h"

void TestBase::run(int num, ...)
{
    debug("Running Tests");
    this->beforeClass();

    va_list ap;
    bool(*f)();
    va_start(ap, num);

    for (int i = 0; i < num; i++) {
        this->setup();

        f = va_arg(ap, bool(*)());

        if (f()) {
            debug("OK");
        } else {
            debug("FAIL");
        }

        this->tearDown();
    }

    va_end(ap);
    this->afterClass();
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
