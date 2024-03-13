//
// Created by dingjing on 24-3-13.
//

#ifndef CLIBRARY_TEST_H
#define CLIBRARY_TEST_H
#include <stdlib.h>
#include <string.h>

typedef enum _CTestStatus   CTestStatus;

enum _CTestStatus
{
    C_TEST_SUCCESS = 0,
    C_TEST_FAILED,
};

double c_test_get_seconds();
void c_test_print(CTestStatus status, const char* format, ...);

#define c_test_true(isTrue, ...) \
do { \
    if (isTrue) { \
        c_test_print (C_TEST_SUCCESS, ## __VA_ARGS__); \
    } else { \
        c_test_print (C_TEST_FAILED, ## __VA_ARGS__); \
    } \
} while(0)

inline void c_test_double(double i1, double i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%f\" == \"%f\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%f\" != \"%f\"", i1, i2);
}

inline void c_test_float(float i1, float i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%f\" == \"%f\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%f\" != \"%f\"", i1, i2);
}

inline void c_test_uint(unsigned int i1, unsigned int i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%ul\" == \"%ul\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%ul\" != \"%ul\"", i1, i2);
}

inline void c_test_int(int i1, int i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%d\" == \"%d\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%d\" != \"%d\"", i1, i2);
}

inline void c_test_long(long i1, long i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%d\" == \"%d\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%d\" != \"%d\"", i1, i2);
}

inline void c_test_ulong(unsigned long i1, unsigned long i2)
{
    if (i1 == i2) {
        c_test_print (C_TEST_SUCCESS, "\"%ul\" == \"%ul\"", i1, i2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%ul\" != \"%ul\"", i1, i2);
}

inline void c_test_str_equal(const char* s1, const char* s2)
{
    if (s1 && s2 && (0 == strcmp (s1, s2))) {
        c_test_print (C_TEST_SUCCESS, "\"%s\" == \"%s\"", s1, s2);
        return;
    }
    c_test_print (C_TEST_FAILED, "\"%s\" != \"%s\"", s1, s2);
}


#endif //CLIBRARY_TEST_H
