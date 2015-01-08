extern "C" {
#include "../../main/ds18x20lib.h"
}

CLASS(Ds18x20LibTest, TestBase)

BEFORECLASS {
    debug("Test BeforeClass");
}

AFTERCLASS {
    debug("Test AfterClass");
}
/*
S ETUP
{
        debug("Test Setup");
}
*/

TEARDOWN {
    debug("Test Teardown");
}

TEST(test1)
{
    debug("Hello from Reset Test");
    reset();
    return true;
}

TEST(test2)
{
    debug("Hello from test2");
    return true;
}

TEST(test3)
{
    debug("Hello from test3");
    return true;
}

RUN

