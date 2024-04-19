
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-19.
//

#include "quark.h"

#include "../c/thread.h"

#define QUARK_BLOCK_SIZE            2048
#define QUARK_STRING_BLOCK_SIZE     (4096 - sizeof (csize))

static inline CQuark quark_new (char* string);

C_LOCK_DEFINE_STATIC (gsQuarkGlobal);

static CHashTable*      gsQuarkHt = NULL;
static char**           gsQuarks = NULL;
static int              gsQuarkSeqID = 0;
static char*            gsQuarkBlock = NULL;
static int              gsQuarkBlockOffset = 0;

void c_quark_init (void)
{
    c_assert (quark_seq_id == 0);
    quark_ht = g_hash_table_new (g_str_hash, g_str_equal);
    quarks = g_new (gchar*, QUARK_BLOCK_SIZE);
    quarks[0] = NULL;
    quark_seq_id = 1;
}
