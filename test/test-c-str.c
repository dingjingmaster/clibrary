
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-3-13.
//
#include "../c/str.h"
#include "../c/test.h"

int main (C_UNUSED int argc, C_UNUSED char* argv[])
{
    char* s1T = "QWERTYUIOPASDFGHJKLZXCVBNM`1234567890-=\\][;'/.,!@#$%^&*()_+|}:?><";
    char* s1K = "qwertyuiopasdfghjklzxcvbnm`1234567890-=\\][;'/.,!@#$%^&*()_+|}:?><";

    int i;
    for (i = 0; s1T[i]; ++i) {
        c_test_true ((c_ascii_tolower (s1T[i]) == s1K[i]), "%c to lower %c == %c", s1T[i], c_ascii_tolower (s1T[i]), s1K[i]);
        c_test_true ((s1T[i] == c_ascii_toupper (s1K[i])), "%c to upper %c == %c", s1K[i], c_ascii_toupper (s1K[i]), s1T[i]);

        if (s1T[i] >= '0' && s1T[i] <= '9') {
            if (-1 != c_ascii_digit_value (s1T[i])) {
                c_test_true(true, "digit %c == %d", s1T[i], c_ascii_digit_value (s1T[i]));
            }
            else {
                c_test_true(false, "digit %c == %d", s1T[i], c_ascii_digit_value (s1T[i]));
            }
        }

        if ((s1T[i] >= '0' && s1T[i] <= '9')
            || (s1T[i] >= 'a' && s1T[i] <= 'f')
            || (s1T[i] >= 'A' && s1T[i] <= 'F')) {
            if (-1 != c_ascii_xdigit_value (s1T[i])) {
                c_test_true(true, "digit %c == %d", s1T[i], c_ascii_xdigit_value (s1T[i]));
            }
            else {
                c_test_true(false, "digit %c == %d", s1T[i], c_ascii_xdigit_value (s1T[i]));
            }
        }
    }

    c_test_true(0 == c_ascii_strcasecmp ("ASDjkl", "ASDJKL"), "ASDjkl == ASDJKL");
    c_test_double (c_ascii_strtod ("1234dasd09", NULL), 1234.0);
    c_test_double (c_ascii_strtod ("xAkdasd09", NULL), 0.0);
    c_test_double (c_ascii_strtod ("0xAkdasd09", NULL), 10.0);

    char buf[32] = {0};
    c_ascii_dtostr (buf, sizeof(buf) - 1, 12);
    c_test_str_equal (buf, "12");
    c_test_str_equal (c_ascii_dtostr (buf, sizeof(buf) - 1, 12), "12");

    char* str21 = "abcdefg";
    c_test_str_equal (c_ascii_strup(str21, strlen (str21)), "ABCDEFG");     // 释放资源
    c_test_str_equal (c_ascii_strdown(str21, strlen (str21)), "abcdefg");   // 释放资源

    c_test_true(0 == c_ascii_strncasecmp("abcdef", "ABCDEFG", 6), "c_ascii_strncasecmp");
    c_test_true(0 > c_ascii_strncasecmp("abcdef", "ABCDEFG", 7), "c_ascii_strncasecmp");

    c_test_true(111 == c_ascii_strtoull("111abcdef", NULL, 10), "c_ascii_strtoull");
    c_test_true(111 == c_ascii_strtoll("111abcdef", NULL, 10), "c_ascii_strtoll");

    c_test_str_equal("10.000000", c_ascii_formatd(buf, sizeof(buf) - 1, "%f", 10));
    c_test_str_equal(NULL, c_ascii_formatd(buf, sizeof(buf) - 1, "%d", 10));

//    c_printf ("%s", "c_printf\n");

    char* str22 = "  abcdef";
    char* str23 = "  abcdef  ";
    char* str24 = " ABCD ";
    char* str25 = "你好";
    char* str26 = "\\\"\\n";
    c_test_str_equal (c_strup(str21), "ABCDEFG");
    c_test_str_equal (c_strchug(str22), "abcdef");

    c_test_str_equal (c_strchomp(str23), "  abcdef");
    c_test_str_equal (c_strreverse(str21), "gfedcba");
    c_test_str_equal (c_strdown(str24), " abcd ");

    c_test_true(c_str_is_ascii(s1T), "c_str_is_ascii");
    c_test_true(c_str_is_ascii(s1K), "c_str_is_ascii");
    c_test_true(!c_str_is_ascii(str25), "c_str_is_ascii");

    c_test_str_equal (c_strcompress (str26), "\"\n");

    c_test_double (10, c_strtod ("10adasdas", NULL));

    return c_test_result();
}