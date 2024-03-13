//
// Created by dingjing on 24-3-13.
//
#include "../c/str.h"
#include "../c/test.h"

int main ()
{
    char* s1T = "QWERTYUIOPASDFGHJKLZXCVBNM`1234567890-=\\][;'/.,!@#$%^&*()_+|}:?><";
    char* s1K = "qwertyuiopasdfghjklzxcvbnm`1234567890-=\\][;'/.,!@#$%^&*()_+|}:?><";

    int i;
    for (i = 0; s1T[i]; ++i) {
        c_test_true ((c_ascii_tolower (s1T[i]) == s1K[i]), "%c to lower %c == %c", s1T[i], c_ascii_tolower (s1T[i]), s1K[i]);
        c_test_true ((s1T[i] == c_ascii_toupper (s1K[i])), "%c to upper %c == %c", s1K[i], c_ascii_toupper (s1K[i]), s1T[i]);
        c_test_true ((c_ascii_digit_value (s1T[i]) && ((s1T[i] >= '0' && s1T[i] <= '9') ? s1T[i] : -1)), "%c == %c", c_ascii_digit_value (s1T[i]), s1T[i]);
    }

    return 0;
}