#define DEBUG

#include "test.h"

extern "C" {
#include "stdio.h"
#include "../../main/debug.h"
#include "../../main/ds18x20lib.h"
}

#include "TestBase.h"


class Ds18x20libTest : public TestBase
{
public:
    void beforeClass();
    void afterClass();
    /*
    void setup();
    void tearDown();
    */
};
void Ds18x20libTest::beforeClass()
{
    debug("Test BeforeClass");
}

void Ds18x20libTest::afterClass()
{
    debug("Test AfterClass");
}
/*
void Ds18x20libTest::setup() {
        debug("Test Setup");
}

void Ds18x20libTest::tearDown() {
        debug("Test Teardown");
}
*/
static bool test1()
{
    debug("Hello from Reset Test");
    reset();
    return false;
}

static bool test2()
{
    debug("Hello from test2");
    return true;
}

int main (void)
{
    Ds18x20libTest t;
    t.run(2, &test1, &test2);
    EXIT_PORT = 1;
}
