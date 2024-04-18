
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
    c_log_init (C_LOG_LEVEL_VERB, 10240, "/tmp/", "a", "log", true);
    C_LOG_DEBUG("1111111");
#endif

    c_log_destroy();

    return 0;
}
