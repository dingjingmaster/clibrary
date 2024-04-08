//
// Created by dingjing on 24-3-18.
//

#ifndef CLIBRARY_BASE64_H
#define CLIBRARY_BASE64_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif
#include "macros.h"

C_BEGIN_EXTERN_C

cuint64 c_base64_encode_step    (const cuchar* in, cuint64 len, bool breakLines, char* out, int* state, int* save);
cuint64 c_base64_encode_close   (bool breakLines, char* out, int* state, int* save);
char*   c_base64_encode         (const cuchar* data, cuint64 len) C_MALLOC;
cuint64 c_base64_decode_step    (const char* in, cuint64 len, cuchar* out, int* state, cuint* save);
cuchar* c_base64_decode         (const char* text, cuint64* outLen) C_MALLOC;
cuchar* c_base64_decode_inplace (char* text, cuint64* outLen);

C_END_EXTERN_C

#endif //CLIBRARY_BASE64_H
