//
// Created by dingjing on 24-3-12.
//

#ifndef CLIBRARY_STR_H
#define CLIBRARY_STR_H
#include "macros.h"

#include <stdio.h>
#include <stdarg.h>

C_BEGIN_EXTERN_C

#define  C_STR_DELIMITERS       "_-|> <."

#define c_ascii_isalnum(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_ALNUM) != 0)
#define c_ascii_isalpha(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_ALPHA) != 0)
#define c_ascii_iscntrl(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_CNTRL) != 0)
#define c_ascii_isdigit(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_DIGIT) != 0)
#define c_ascii_isgraph(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_GRAPH) != 0)
#define c_ascii_islower(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_LOWER) != 0)
#define c_ascii_isprint(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_PRINT) != 0)
#define c_ascii_ispunct(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_PUNCT) != 0)
#define c_ascii_isspace(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_SPACE) != 0)
#define c_ascii_isupper(c)      ((gpAsciiTable[(cuchar) (c)] & C_ASCII_UPPER) != 0)
#define c_ascii_isxdigit(c)     ((gpAsciiTable[(cuchar) (c)] & C_ASCII_XDIGIT) != 0)


typedef enum
{
    C_ASCII_ALNUM  = 1 << 0,
    C_ASCII_ALPHA  = 1 << 1,
    C_ASCII_CNTRL  = 1 << 2,
    C_ASCII_DIGIT  = 1 << 3,
    C_ASCII_GRAPH  = 1 << 4,
    C_ASCII_LOWER  = 1 << 5,
    C_ASCII_PRINT  = 1 << 6,
    C_ASCII_PUNCT  = 1 << 7,
    C_ASCII_SPACE  = 1 << 8,
    C_ASCII_UPPER  = 1 << 9,
    C_ASCII_XDIGIT = 1 << 10
} CAsciiType;

C_SYMBOL_PROTECTED extern const cuint16* const gpAsciiTable;

char                  c_ascii_tolower       (char c) C_CONST;                           // OK
char                  c_ascii_toupper       (char c) C_CONST;                           // OK
int                   c_ascii_digit_value   (char c) C_CONST;                           // OK
int                   c_ascii_xdigit_value  (char c) C_CONST;                           // OK
int                   c_ascii_strcasecmp    (const char* s1, const char* s2);           // OK
double                c_ascii_strtod        (const char* nPtr, char** endPtr);          // OK
char*                 c_ascii_dtostr        (char* buffer, int bufLen, double d);       // OK
char*                 c_ascii_strdown       (const char* str, cuint64 len) C_MALLOC;    // OK
char*                 c_ascii_strup         (const char* str, cuint64 len) C_MALLOC;    // OK
int                   c_ascii_strncasecmp   (const char* s1, const char* s2, cint64 n); // OK
cuint64               c_ascii_strtoull      (const char* nPtr, char** endPtr, cuint base);      // OK
cint64                c_ascii_strtoll       (const char* nPtr, char** endPtr, cuint base);      // OK
char*                 c_ascii_formatd       (char* buffer, int bufLen, const char* format, double d);   // OK

int                   c_printf              (char const* format, ...) C_PRINTF (1, 2);                  // OK
int                   c_vprintf             (char const* format, va_list args) C_PRINTF(1, 0);          // OK
int                   c_sprintf             (char* str, char const* format, ...) C_PRINTF (2, 3);       // OK
int                   c_fprintf             (FILE* file, char const *format, ...) C_PRINTF (2, 3);      // OK
int                   c_vsprintf            (char* str, char const* format, va_list args) C_PRINTF(2, 0);   // OK
int                   c_vfprintf            (FILE* file, char const* format, va_list args) C_PRINTF(2, 0);  // OK
int                   c_vasprintf           (char** str, char const* format, va_list args) C_PRINTF(2, 0);  // OK
int                   c_snprintf            (char* str, culong n, char const *format, ...);             // OK
int                   c_vsnprintf           (char* str, culong n, char const* format, va_list args);    // OK

cuint64               c_printf_string_upper_bound (const char* format, va_list args) C_PRINTF(1, 0);

char*                 c_strup               (char* str);                            // OK
char*                 c_strchug             (char* str);                            // OK
char*                 c_strchomp            (char* str);                            // OK
char*                 c_strreverse          (char* str);                            // OK
char*                 c_strdown             (char* str);                            // OK
bool                  c_str_is_ascii        (const char* str);                      // OK
void                  c_strfreev            (char** strArray);                      // OK
char**                c_strdupv             (char** strArray);                      // OK
cuint                 c_strv_length         (char** strArray);                      // OK
const char*           c_strerror            (int errNum) C_CONST;                   // OK
const char*           c_strsignal           (int signum) C_CONST;                   // OK
char*                 c_strdup              (const char* str) C_MALLOC;             // OK
char*                 c_stpcpy              (char* dest, const char* src);          // OK
char*                 c_strcompress         (const char* source) C_MALLOC;          // OK
double                c_strtod              (const char* nPtr, char** endPtr);      // OK
char*                 c_strndup             (const char* str, cuint64 n) C_MALLOC;  // OK
char*                 c_strnfill            (cint64 length, char fillChar) C_MALLOC;                        // OK
char*                 c_strrstr             (const char* haystack, const char* needle);                     // OK
cint64                c_strlcpy             (char* dest, const char* src, cint64 destSize);                 // OK
cint64                c_strlcat             (char* dest, const char* src, cint64 destSize);                 // OK
char*                 c_strjoinv            (const char* separator, char** strArray) C_MALLOC;              // OK
char*                 c_strconcat           (const char* str1, ...) C_MALLOC C_NULL_TERMINATED;             // OK
char*                 c_strdup_printf       (const char* format, ...) C_PRINTF (1, 2) C_MALLOC;             // OK
char*                 c_strescape           (const char* source, const char* exceptions) C_MALLOC;          // OK
char*                 c_strcanon            (char* str, const char* validChars, char substitutor);          // OK
char*                 c_strdelimit          (char* str, const char* delimiters, char newDelimiter);         // OK
char*                 c_strjoin             (const char* separator, ...) C_MALLOC C_NULL_TERMINATED;        // OK
char**                c_strsplit            (const char* str, const char* delimiter, int maxTokens);        // OK
char**                c_strsplit_set        (const char* str, const char* delimiters, int maxTokens);       // OK
char*                 c_strdup_vprintf      (const char* format, va_list args) C_PRINTF(1, 0) C_MALLOC;     // OK
char*                 c_strrstr_len         (const char* haystack, cuint64 haystackLen, const char* needle); // OK
char*                 c_strstr_len          (const char* haystack, cuint64 haystackLen, const char* needle); // OK

bool                  c_str_has_suffix      (const char* str, const char* suffix);  // OK
bool                  c_str_has_prefix      (const char* str, const char* prefix);  // OK

C_END_EXTERN_C

#endif //CLIBRARY_STR_H
