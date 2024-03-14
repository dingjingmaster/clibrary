//
// Created by dingjing on 24-3-13.
//

#include "test.h"

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define COLOR_FAILED        "\033[1;31m"
#define COLOR_SUCCESS       "\033[1;32m"
#define COLOR_NONE          "\033[0m"

static unsigned long        gsSuccess;
static unsigned long        gsFailed;

double c_test_get_seconds()
{
#ifdef _MSC_VER
    LARGE_INTEGER freq, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);

    return (double) now.QuadPart / freq.QuadPart;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 1e-9;
#endif
}

void c_test_print(CTestStatus status, const char *format, ...)
{
    char message[4096];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof (message) - 1, format, args);
    va_end(args);

    (status == C_TEST_SUCCESS) ? ++gsSuccess : ++gsFailed;

    printf ("%s[%s] %s %s\n", ((status == C_TEST_SUCCESS) ? COLOR_SUCCESS : COLOR_FAILED),
            ((status == C_TEST_SUCCESS) ? "SUCCESS" : "FAILED "), message, COLOR_NONE);
}

int c_test_result()
{
    printf ("\033[35mTest result: [SUCCESS]: %lu  [FAILED ]: %lu\033[0m", gsSuccess, gsFailed);

    return -gsFailed;
}
