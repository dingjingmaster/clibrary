
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
    CHashTable* hash1 = c_hash_table_new_full(c_str_hash, c_str_equal, c_free0, c_free0);

    cuint i = 9999999;

    while (i--) {
        char *k1 = c_strdup("k1");
        char *v1 = c_strdup("v1");
        if (!c_hash_table_contains(hash1, k1)) {
            c_hash_table_insert(hash1, k1, v1);
        }
        else {
            c_free(k1);
            c_free(v1);
        }

        char *k2 = c_strdup("k2");
        char *v2 = c_strdup("v2");
        if (!c_hash_table_contains(hash1, k2)) {
            c_hash_table_insert(hash1, k2, v2);
        }
        else {
            c_free(k2);
            c_free(v2);
        }

        char *k3 = c_strdup("k3");
        char *v3 = c_strdup("v3");
        if (!c_hash_table_contains(hash1, k3)) {
            c_hash_table_insert(hash1, k3, v3);
        }
        else {
            c_free(k3);
            c_free(v3);
        }

        char *k4 = c_strdup("k4");
        char *v4 = c_strdup("v4");
        if (!c_hash_table_contains(hash1, k4)) {
            c_hash_table_insert(hash1, k4, v4);
        }
        else {
            c_free(k4);
            c_free(v4);
        }

        char *k5 = c_strdup("k5");
        char *v5 = c_strdup("v5");
        if (!c_hash_table_contains(hash1, k5)) {
            c_hash_table_insert(hash1, k5, v5);
        }
        else {
            c_free(k5);
            c_free(v5);
        }

        char *k6 = c_strdup("k6");
        char *v6 = c_strdup("v6");
        if (!c_hash_table_contains(hash1, k6)) {
            c_hash_table_insert(hash1, k6, v6);
        }
        else {
            c_free(k6);
            c_free(v6);
        }

        char *k7 = c_strdup("k7");
        char *v7 = c_strdup("v7");
        if (!c_hash_table_contains(hash1, k7)) {
            c_hash_table_insert(hash1, k7, v7);
        }
        else {
            c_free(k7);
            c_free(v7);
        }

        char *k8 = c_strdup("k8");
        char *v8 = c_strdup("v8");
        if (!c_hash_table_contains(hash1, k8)) {
            c_hash_table_insert(hash1, k8, v8);
        }
        else {
            c_free(k8);
            c_free(v8);
        }


        char buf[] = "/qwqertyuio,/asdadqwczhji,/asdiuhiads";
        char** p = c_strsplit(buf, ",", -1);
        if (c_strv_length(p) < 2) {
            c_strfreev(p);
            continue;
        }

        char* p1f = c_file_path_format_arr(p[0]);
        char* p2f = c_file_path_format_arr(p[1]);

        if (!c_hash_table_contains(hash1, p1f)) {
            c_hash_table_insert(hash1, p1f, p2f);
        }
    }
    return 0;
}