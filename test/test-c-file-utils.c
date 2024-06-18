// Copyright (c) 2024. Lorem ipsum dolor sit amet, consectetur adipiscing elit.
// Morbi non lorem porttitor neque feugiat blandit. Ut vitae ipsum eget quam lacinia accumsan.
// Etiam sed turpis ac ipsum condimentum fringilla. Maecenas magna.
// Proin dapibus sapien vel ante. Aliquam erat volutpat. Pellentesque sagittis ligula eget metus.
// Vestibulum commodo. Ut rhoncus gravida arcu.

//
// Created by dingjing on 6/17/24.
//

#include <c/clib.h>

#include "c/test.h"

int main (int C_UNUSED argc, char* C_UNUSED argv[])
{
    char file1[] = "/////////a/b/c/d/e/f/";
    char file2[] = "/////////a///b///c/////d/////e/////f/";
    char file3[] = "/////////a/b/c/d/e/f///////////////////////";

    char* file11 = c_file_path_format_arr(file1);
    c_test_str_equal(file1, file11);
    char* file22 = c_file_path_format_arr(file2);
    c_test_str_equal(file2, file22);
    char* file33 = c_file_path_format_arr(file3);
    c_test_str_equal(file3, file33);

    return c_test_result();
}
