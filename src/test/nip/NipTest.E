# 1 "src/test/nip/NipTest.cpp"
# 1 "/home/werner/src/homeserver//"
# 1 "<command-line>"
# 1 "src/test/nip/NipTest.cpp"
# 1 "src/test/nip/../test.h" 1
# 9 "src/test/nip/../test.h"
extern "C" {


# 1 "/usr/lib/gcc/avr/4.8.1/include/stdint.h" 1 3 4
# 9 "/usr/lib/gcc/avr/4.8.1/include/stdint.h" 3 4
# 1 "/usr/lib/avr/include/stdint.h" 1 3 4
# 121 "/usr/lib/avr/include/stdint.h" 3 4
typedef signed int int8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef signed int int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef signed int int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));

typedef signed int int64_t __attribute__((__mode__(__DI__)));
typedef unsigned int uint64_t __attribute__((__mode__(__DI__)));
# 142 "/usr/lib/avr/include/stdint.h" 3 4
typedef int16_t intptr_t;




typedef uint16_t uintptr_t;
# 159 "/usr/lib/avr/include/stdint.h" 3 4
typedef int8_t int_least8_t;




typedef uint8_t uint_least8_t;




typedef int16_t int_least16_t;




typedef uint16_t uint_least16_t;




typedef int32_t int_least32_t;




typedef uint32_t uint_least32_t;







typedef int64_t int_least64_t;






typedef uint64_t uint_least64_t;
# 213 "/usr/lib/avr/include/stdint.h" 3 4
typedef int8_t int_fast8_t;




typedef uint8_t uint_fast8_t;




typedef int16_t int_fast16_t;




typedef uint16_t uint_fast16_t;




typedef int32_t int_fast32_t;




typedef uint32_t uint_fast32_t;







typedef int64_t int_fast64_t;






typedef uint64_t uint_fast64_t;
# 273 "/usr/lib/avr/include/stdint.h" 3 4
typedef int64_t intmax_t;




typedef uint64_t uintmax_t;
# 10 "/usr/lib/gcc/avr/4.8.1/include/stdint.h" 2 3 4
# 13 "src/test/nip/../test.h" 2


}
# 2 "src/test/nip/NipTest.cpp" 2
# 1 "src/test/nip/../mock.h" 1






extern "C" {



}



typedef struct {
    uint8_t type;
    uint32_t value;
} event_t;



extern "C" {

void initMock();
void event(event_t*);
int cmpMock(uint16_t);

}
# 3 "src/test/nip/NipTest.cpp" 2

# 1 "src/test/nip/../TestBase.h" 1



class TestBase
{
public:
    int run(int, ...);
    virtual void setup();
    virtual void tearDown();
    virtual void beforeClass();
    virtual void afterClass();

private:
    int testResult;
};
# 5 "src/test/nip/NipTest.cpp" 2

class NipTest : public TestBase
{
public:
void beforeClass();

void setup();

};






extern "C" {
# 1 "src/test/nip/../../main/debug.h" 1
# 20 "src/test/nip/../../main/debug.h"
void debug_c(unsigned char);
int debug_printf (char const*, ...);
# 22 "src/test/nip/NipTest.cpp" 2

# 1 "src/test/nip/../../main/arp.h" 1






void arp_insert(uint8_t* ip, uint8_t* mac);




void arp_request(uint8_t* ip, uint8_t* mac);






void arp_lookup(uint8_t* ip, uint8_t* mac);
# 24 "src/test/nip/NipTest.cpp" 2
# 1 "/usr/lib/avr/include/string.h" 1 3
# 45 "/usr/lib/avr/include/string.h" 3
# 1 "/usr/lib/gcc/avr/4.8.1/include/stddef.h" 1 3 4
# 212 "/usr/lib/gcc/avr/4.8.1/include/stddef.h" 3 4
typedef unsigned int size_t;
# 46 "/usr/lib/avr/include/string.h" 2 3
# 56 "/usr/lib/avr/include/string.h" 3
extern "C" {
# 111 "/usr/lib/avr/include/string.h" 3
extern int ffs (int __val) __attribute__((__const__));
extern int ffsl (long __val) __attribute__((__const__));
extern int ffsll (long long __val) __attribute__((__const__));
extern void *memccpy(void *, const void *, int, size_t);
extern void *memchr(const void *, int, size_t) __attribute__((__pure__));
extern int memcmp(const void *, const void *, size_t) __attribute__((__pure__));
extern void *memcpy(void *, const void *, size_t);
extern void *memmem(const void *, size_t, const void *, size_t) __attribute__((__pure__));
extern void *memmove(void *, const void *, size_t);
extern void *memrchr(const void *, int, size_t) __attribute__((__pure__));
extern void *memset(void *, int, size_t);
extern char *strcat(char *, const char *);
extern char *strchr(const char *, int) __attribute__((__pure__));
extern char *strchrnul(const char *, int) __attribute__((__pure__));
extern int strcmp(const char *, const char *) __attribute__((__pure__));
extern char *strcpy(char *, const char *);
extern int strcasecmp(const char *, const char *) __attribute__((__pure__));
extern char *strcasestr(const char *, const char *) __attribute__((__pure__));
extern size_t strcspn(const char *__s, const char *__reject) __attribute__((__pure__));
extern char *strdup(const char *s1);
extern size_t strlcat(char *, const char *, size_t);
extern size_t strlcpy(char *, const char *, size_t);
extern size_t strlen(const char *) __attribute__((__pure__));
extern char *strlwr(char *);
extern char *strncat(char *, const char *, size_t);
extern int strncmp(const char *, const char *, size_t) __attribute__((__pure__));
extern char *strncpy(char *, const char *, size_t);
extern int strncasecmp(const char *, const char *, size_t) __attribute__((__pure__));
extern size_t strnlen(const char *, size_t) __attribute__((__pure__));
extern char *strpbrk(const char *__s, const char *__accept) __attribute__((__pure__));
extern char *strrchr(const char *, int) __attribute__((__pure__));
extern char *strrev(char *);
extern char *strsep(char **, const char *);
extern size_t strspn(const char *__s, const char *__accept) __attribute__((__pure__));
extern char *strstr(const char *, const char *) __attribute__((__pure__));
extern char *strtok(char *, const char *);
extern char *strtok_r(char *, const char *, char **);
extern char *strupr(char *);


}
# 25 "src/test/nip/NipTest.cpp" 2
}


void NipTest::beforeClass()
{
}


void NipTest::setup()
{
}


static bool arpInsert()
{

 debug_printf("3 %s:%d: " "inserting" "\n\r","src/test/nip/NipTest.cpp",41);
 uint8_t tmp_mac2[6];
 uint8_t tmp_ip2[4];
 tmp_mac2=(uint8_t*)(uint8_t[]){10,20,30,40,50,60};
 tmp_ip2=(uint8_t*)(uint8_t[]){10,20,30,40};
 arp_insert(tmp_ip2,tmp_mac2);
 arp_insert(tmp_ip2,tmp_mac2);
    arp_insert((uint8_t*)(uint8_t[]){10,20,30,40},(uint8_t*)(uint8_t[]){10,20,30,40,50,60});
    arp_insert((uint8_t*)(uint8_t[]){1,2,3,4},(uint8_t*)(uint8_t[]){1,2,3,4,5,6});
    arp_insert((uint8_t*)(uint8_t[]){1,2,3,4},(uint8_t*)(uint8_t[]){2,2,3,4,5,6});
    arp_insert((uint8_t*)(uint8_t[]){2,2,3,4},(uint8_t*)(uint8_t[]){1,2,3,4,5,6});
 debug_printf("3 %s:%d: " "inserting end" "\n\r","src/test/nip/NipTest.cpp",52);


 uint8_t tmp_ip[4]= {};
 uint8_t tmp_mac[6] = {};
 for (uint8_t i=1;i<1;i++)
  arp_insert((uint8_t*)(uint8_t[]){i,1,1,1},(uint8_t*)(uint8_t[]){i,1,1,1,1,1});


 arp_lookup(tmp_ip,tmp_mac);
    for (int i = 0; i < 6; i++) {
        debug_printf("%02x ", tmp_mac[i]);
    }


    arp_lookup((uint8_t*)(uint8_t[]){1,2,3,4}, tmp_mac);

    for (int i = 0; i < 6; i++) {
        debug_printf("%02x ", tmp_mac[i]);
    }

    debug_printf( "\n\r");

    for (int i = 0; i < 6; i++) {
        debug_printf("%02x ", tmp_mac[i]);
    }

    debug_printf( "\n\r");

    return ! strncmp((char*)tmp_mac, (char*)tmp_mac, 6);
}


int main (void)
{
    NipTest t;
    (*((volatile char *)0x21)) = ! t.run(1,&arpInsert);
}
