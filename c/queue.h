
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 5/1/24.
//

#ifndef CLIBRARY_QUEUE_H
#define CLIBRARY_QUEUE_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif

#include <c/poll.h>
#include <c/list.h>
#include <c/macros.h>
#include <c/thread.h>

C_BEGIN_EXTERN_C

typedef struct _CQueue CQueue;

struct _CQueue
{
    CList*  head;
    CList*  tail;
    cuint   length;
};

#define C_QUEUE_INIT { NULL, NULL, 0 }

CQueue*  c_queue_new                (void);
void     c_queue_free               (CQueue* queue);
void     c_queue_free_full          (CQueue* queue, CDestroyNotify freeFunc);
void     c_queue_init               (CQueue* queue);
void     c_queue_clear              (CQueue* queue);
bool     c_queue_is_empty           (CQueue* queue);
void     c_queue_clear_full         (CQueue* queue, CDestroyNotify freeFunc);
cuint    c_queue_get_length         (CQueue* queue);
void     c_queue_reverse            (CQueue* queue);
CQueue*  c_queue_copy               (CQueue* queue);
void     c_queue_foreach            (CQueue* queue, CFunc func, void* udata);
CList*   c_queue_find               (CQueue* queue, const void* data);
CList*   c_queue_find_custom        (CQueue* queue, const void* data, CCompareFunc func);
void     c_queue_sort               (CQueue* queue, CCompareDataFunc compareFunc, void* udata);
void     c_queue_push_head          (CQueue* queue, void* data);
void     c_queue_push_tail          (CQueue* queue, void* data);
void     c_queue_push_nth           (CQueue* queue, void* data, cint n);
void*    c_queue_pop_head           (CQueue* queue);
void*    c_queue_pop_tail           (CQueue* queue);
void*    c_queue_pop_nth            (CQueue* queue, cuint n);
void*    c_queue_peek_head          (CQueue* queue);
void*    c_queue_peek_tail          (CQueue* queue);
void*    c_queue_peek_nth           (CQueue* queue, cuint n);
cint     c_queue_index              (CQueue* queue, const void* data);
bool     c_queue_remove             (CQueue* queue, const void* data);
cuint    c_queue_remove_all         (CQueue* queue, const void* data);
void     c_queue_insert_before      (CQueue* queue, CList* sibling, void* data);
void     c_queue_insert_before_link (CQueue* queue, CList* sibling, CList* link_);
void     c_queue_insert_after       (CQueue* queue, CList* sibling, void* data);
void     c_queue_insert_after_link  (CQueue* queue, CList* sibling, CList* link_);
void     c_queue_insert_sorted      (CQueue* queue, void* data, CCompareDataFunc func, void* udata);
void     c_queue_push_head_link     (CQueue* queue, CList* link_);
void     c_queue_push_tail_link     (CQueue* queue, CList* link_);
void     c_queue_push_nth_link      (CQueue* queue, cint n, CList* link_);
CList*   c_queue_pop_head_link      (CQueue* queue);
CList*   c_queue_pop_tail_link      (CQueue* queue);
CList*   c_queue_pop_nth_link       (CQueue* queue, cuint n);
CList*   c_queue_peek_head_link     (CQueue* queue);
CList*   c_queue_peek_tail_link     (CQueue* queue);
CList*   c_queue_peek_nth_link      (CQueue* queue, cuint n);
cint     c_queue_link_index         (CQueue* queue, CList* link_);
void     c_queue_unlink             (CQueue* queue, CList* link_);
void     c_queue_delete_link        (CQueue* queue, CList* link_);

C_END_EXTERN_C


#endif // CLIBRARY_QUEUE_H
