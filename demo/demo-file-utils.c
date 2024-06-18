
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-6-18.
//

#include <c/clib.h>

int main (int C_UNUSED argc, char* C_UNUSED argv[])
{
#define FILE_PATH "/tmp/aa.txt"
    if (c_file_test(FILE_PATH, C_FILE_TEST_EXISTS)) {
        char buf[10] = {0};
        FILE* fr = NULL;
        do {
            cuint64 len = 1;
            fr = c_fopen(FILE_PATH, "r");
            if (fr) {
                while (len > 0) {
                    len = c_file_read_line_arr(fr, buf, sizeof(buf) - 1);
                    printf("%s\n", buf);
                }
            }
        } while (false);
        fclose(fr);
    }

    return 0;
}