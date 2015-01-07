
#include "test.h"
#include "TestBase.h"

extern "C" {
#include "../../main/ds18x20lib.h"
}


class Ds18x20libTest : public TestBase 
{
public:
    void beforeClass();
    void afterClass();
    void setup();
    void tearDown();
};

void Ds18x20libTest::beforeClass()
{
    debug("Test BeforeClass");
}

void Ds18x20libTest::afterClass()
{
    debug("Test AfterClass");
}

void Ds18x20libTest::setup() 
{
        debug("Test Setup");
}

void Ds18x20libTest::tearDown() 
{
        debug("Test Teardown");
}

static bool test1()
{
    debug("Hello from Reset Test");
    reset();
    return true;
}

static bool test2()
{
    debug("Hello from test2");
    return true;
}

static bool test3() 
{
    debug("Hello from test3");
    return true;
}


int main (void)
{
    Ds18x20libTest t;
    //invert for exit-code
    EXIT_PORT = ! t.run(3,&test1,&test2,&test3);
}
