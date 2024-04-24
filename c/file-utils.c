
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-24.
//

#include "file-utils.h"

#include <stdarg.h>
#include <sys/stat.h>

#include "str.h"
#include "cstring.h"


static char* c_build_filename_va (const char* firstArgument, va_list* args, char** strArray);
static char* c_build_path_va (const char* separator, const char* firstElement, va_list* args, char** strArray);


bool c_file_test (const char* fileName, CFileTest test)
{
    c_return_val_if_fail (fileName != NULL, false);

    if ((test & C_FILE_TEST_EXISTS) && (access (fileName, F_OK) == 0)) {
        return true;
    }

    if ((test & C_FILE_TEST_IS_EXECUTABLE) && (access (fileName, X_OK) == 0)) {
        if (getuid () != 0) {
            return true;
        }
    }
    else {
        test &= ~C_FILE_TEST_IS_EXECUTABLE;
    }

    if (test & C_FILE_TEST_IS_SYMLINK) {
        struct stat s;
        if ((lstat (fileName, &s) == 0) && S_ISLNK (s.st_mode)) {
            return true;
        }
    }

    if (test & (C_FILE_TEST_IS_REGULAR | C_FILE_TEST_IS_DIR | C_FILE_TEST_IS_EXECUTABLE)) {
        struct stat s;
        if (stat (fileName, &s) == 0) {
            if ((test & C_FILE_TEST_IS_REGULAR) && S_ISREG (s.st_mode)) {
                return true;
            }

            if ((test & C_FILE_TEST_IS_DIR) && S_ISDIR (s.st_mode)) {
                return true;
            }

            if ((test & C_FILE_TEST_IS_EXECUTABLE) && ((s.st_mode & S_IXOTH) || (s.st_mode & S_IXUSR) || (s.st_mode & S_IXGRP))) {
                return true;
            }
        }
    }

    return false;
}

char* c_build_filename (const char* firstElement, ...)
{
    va_list args;

    va_start (args, firstElement);
    char* str = c_build_filename_va (firstElement, &args, NULL);
    va_end (args);

    return str;
}

char* c_build_filenamev (char** args)
{
    return c_build_filename_va (NULL, NULL, args);
}

char* c_build_filename_valist (const char* firstElement, va_list* args)
{
    c_return_val_if_fail (firstElement != NULL, NULL);

    return c_build_filename_va (firstElement, args, NULL);
}

bool c_path_is_absolute (const char* fileName)
{
    c_return_val_if_fail (fileName != NULL, false);

    if (C_IS_DIR_SEPARATOR (fileName[0])) {
        return true;
    }

    return false;
}


static char* c_build_filename_va (const char* firstArgument, va_list* args, char** strArray)
{
    char* str = c_build_path_va (C_DIR_SEPARATOR_S, firstArgument, args, strArray);

    return str;
}

static char* c_build_path_va (const char* separator, const char* firstElement, va_list* args, char** strArray)
{
    CString *result;
    int separatorLen = (int) strlen (separator);
    bool isFirst = true;
    bool haveLeading = false;
    const char* singleElement = NULL;
    const char* nextElement;
    const char* lastTrailing = NULL;
    int i = 0;

    result = c_string_new (NULL);

    if (strArray) {
        nextElement = strArray[i++];
    }
    else {
        nextElement = firstElement;
    }

    while (true) {
        const char* element;
        const char* start;
        const char* end;

        if (nextElement) {
            element = nextElement;
            if (strArray) {
                nextElement = strArray[i++];
            }
            else {
                nextElement = va_arg (*args, char*);
            }
        }
        else {
            break;
        }

        if (!*element) {
            continue;
        }

        start = element;
        if (separatorLen) {
            while (strncmp (start, separator, separatorLen) == 0) {
                start += separatorLen;
            }
        }

        end = start + strlen (start);
        if (separatorLen) {
            while (end >= start + separatorLen && strncmp (end - separatorLen, separator, separatorLen) == 0) {
                end -= separatorLen;
            }

            lastTrailing = end;
            while (lastTrailing >= element + separatorLen && strncmp (lastTrailing - separatorLen, separator, separatorLen) == 0) {
                lastTrailing -= separatorLen;
            }

            if (!haveLeading) {
                if (lastTrailing <= start) {
                    singleElement = element;
                }

                c_string_append_len (result, element, start - element);
                haveLeading = true;
            }
            else {
                singleElement = NULL;
            }
        }

        if (end == start) {
            continue;
        }

        if (!isFirst) {
            c_string_append (result, separator);
        }

        c_string_append_len (result, start, end - start);
        isFirst = false;
    }

    if (singleElement) {
        c_string_free (result, true);
        return c_strdup (singleElement);
    }
    else {
        if (lastTrailing) {
            c_string_append (result, lastTrailing);
        }
        return c_string_free (result, false);
    }
}

