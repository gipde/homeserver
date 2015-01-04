#ifndef _TESTBASE_H_
#define _TESTBASE_H_

class TestBase
{
public:
    void run(int, ...);
    virtual void setup();
    virtual void tearDown();
    virtual void beforeClass();
    virtual void afterClass();

};

#endif

