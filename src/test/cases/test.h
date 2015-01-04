#ifndef _TEST_H_
#define _TEST_H_

#define EXIT_PORT (*((volatile char *)0x21))

#define TEST(a) TESTMETHODS
#define TESTCASE(clazz,base)    class clazz : public base { \
                                    public: \
                                        void setup(); \
                                        void tearDown(); \
                                };


#define RUNTESTS(...)   int main(int argc, char** argv) { a t;  t.run(__VA_ARGS__); }
#endif
