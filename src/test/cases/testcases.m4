divert(-1)

define(CASE, `define(CLASSNAME,$1)' `define(BASE,$2)') 

define(BEFORECLASS,void CLASSNAME::beforeClass() `define(BEFORECLASS_U)')
define(AFTERCLASS,void CLASSNAME::afterClass() `define(AFTERCLASS_U)')
define(SETUP,void CLASSNAME::setup() `define(SETUP_U)')
define(TEARDOWN,void CLASSNAME::tearDown() `define(TEARDOWN_U)')
define(TESTS,`0')
define(TEST,static bool $1() `ifdef(`TESTNAMES',`define(`TESTNAMES',TESTNAMES; &$1) define(`TESTS',incr(TESTS))',`define(`TESTNAMES',&$1) define(`TESTS',incr(TESTS))')')

divert(2)dnl
include(FILE)
divert(1)dnl
ifdef(`CLASSNAME',
#include "test.h" 
#include "mock.h"
`#'include "BASE.h"

class CLASSNAME : public BASE
{
public:
ifdef(`BEFORECLASS_U',void beforeClass();)
ifdef(`AFTERCLASS_U',void afterClass();)
ifdef(`SETUP_U',void setup();)
ifdef(`TEARDOWN_U',void tearDown();)
};
)
divert(2)
int main (void)
{
    CLASSNAME t;
    EXIT_PORT = ! t.run(TESTS,patsubst(TESTNAMES,`;',`,'));
}
