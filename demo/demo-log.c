//
// Created by dingjing on 24-3-7.
//
#include "../c/log.h"

#define DEMO_FILE 1

int main (C_UNUSED int argc, C_UNUSED char* argv[])
{
#if !DEMO_FILE
    c_log_init (C_LOG_TYPE_CONSOLE, C_LOG_LEVEL_VERB, 10240, "/tmp/", "a", "log", true);

    C_LOG_DEBUG("1111111");
#else
    c_log_init (C_LOG_TYPE_FILE, C_LOG_LEVEL_VERB, 10240, "/tmp/", "a", "log", true);
    C_LOG_DEBUG("1111111");
#endif

    c_log_destroy();

    return 0;
}
