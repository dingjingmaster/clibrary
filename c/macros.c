
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-9.
//

#include "macros.h"

#include "log.h"


typedef struct _MSortParam          MSortParam;

struct _MSortParam
{
    cuint64                 s;
    cuint64                 var;
    CCompareDataFunc        cmp;
    void*                   arg;
    char*                   t;
};


static void msort_with_tmp (const MSortParam* p, void* b, size_t n);
static void msort_r (void *b, cuint64 n, cuint64 s, CCompareDataFunc cmp, void *arg);

void c_abort (void)
{
    C_LOG_WARNING_CONSOLE("c_abort!")

    abort();
}

void c_qsort_with_data (const void* pBase, cint totalElems, csize size, CCompareDataFunc compareFunc, void* udata)
{
    msort_r ((void*) pBase, totalElems, size, compareFunc, udata);
}

bool c_direct_equal (const void* p1, const void* p2)
{
    return p1 == p2;
}

bool c_str_equal (const void* p1, const void* p2)
{
    return 0 == strcmp (p1, p2);
}

bool c_int_equal (const void* p1, const void* p2)
{
    return *(cint*) p1 == *(cint*) p2;
}

bool c_int64_equal (const void* p1, const void* p2)
{
    return *(cint64*) p1 == *(cint64*) p2;
}

bool c_double_equal (const void* p1, const void* p2)
{
    return *(cdouble*) p1 == *(cdouble*) p2;
}


CRand* c_rand_new_with_seed (cuint32  seed)
{}

CRand* c_rand_new_with_seed_array (const cuint32 *seed, cuint seedLength)
{}

CRand* c_rand_new (void)
{}

void c_rand_free (CRand* rand)
{}

CRand* c_rand_copy (CRand* rand)
{}

void c_rand_set_seed (CRand* rand, cuint32 seed)
{}

void c_rand_set_seed_array (CRand* rand, const cuint32* seed, cuint seedLength)
{}

cuint32 c_rand_int (CRand* rand)
{}

cint32 c_rand_int_range (CRand* rand, cint32 begin, cint32 end)
{}

cdouble c_rand_double (CRand* rand)
{}

cdouble c_rand_double_range (CRand* rand, cdouble begin, cdouble end)
{}

void c_random_set_seed (cuint32 seed)
{}

cuint32 c_random_int (void)
{}

cint32 c_random_int_range (cint32 begin, cint32 end)
{}

cdouble c_random_double (void)
{}

cdouble c_random_double_range (cdouble begin, cdouble end)
{}


static void msort_r (void *b, cuint64 n, cuint64 s, CCompareDataFunc cmp, void *arg)
{
    MSortParam p;
    cuint64 size = n * s;

    /* For large object sizes use indirect sorting.  */
    if (s > 32) {
        size = 2 * n * sizeof (void *) + s;
    }

    char* tmp = c_malloc0 (size);
    p.t = tmp;

    p.s = s;
    p.var = 4;
    p.cmp = cmp;
    p.arg = arg;

    if (s > 32) {
        /* Indirect sorting.  */
        char *ip = (char *) b;
        void **tp = (void **) (p.t + n * sizeof (void *));
        void **t = tp;
        void *tmp_storage = (void *) (tp + n);
        char *kp;
        size_t i;

        while ((void *) t < tmp_storage) {
            *t++ = ip;
            ip += s;
        }
        p.s = sizeof (void *);
        p.var = 3;
        msort_with_tmp (&p, p.t + n * sizeof (void *), n);

        for (i = 0, ip = (char *) b; i < n; i++, ip += s) {
            if ((kp = tp[i]) != ip) {
                size_t j = i;
                char *jp = ip;
                memcpy (tmp_storage, ip, s);
                do {
                    size_t k = (kp - (char *) b) / s;
                    tp[j] = jp;
                    memcpy (jp, kp, s);
                    j = k;
                    jp = kp;
                    kp = tp[k];
                } while (kp != ip);

                tp[j] = jp;
                memcpy (jp, tmp_storage, s);
            }
        }
    }
    else {
        if (((s & (sizeof (cuint32) - 1)) == 0) && ((cuint64) b % ALIGNOF_CUINT32 == 0)) {
            if (s == sizeof (cuint32)) {
                p.var = 0;
            }
            else if ((s == sizeof (cuint64)) && ((cuint64) b % ALIGNOF_CUINT64 == 0)) {
                p.var = 1;
            }
            else if (((s & (sizeof (unsigned long) - 1)) == 0) && ((cuint64) b % ALIGNOF_UNSIGNED_LONG == 0)) {
                p.var = 2;
            }
        }
        msort_with_tmp (&p, b, n);
    }
    c_free (tmp);
}

static void msort_with_tmp (const MSortParam* p, void *b, size_t n)
{
    char *b1, *b2;
    size_t n1, n2;
    char *tmp = p->t;
    const size_t s = p->s;
    CCompareDataFunc cmp = p->cmp;
    void *arg = p->arg;

    if (n <= 1) {
        return;
    }

    n1 = n / 2;
    n2 = n - n1;
    b1 = b;
    b2 = (char *) b + (n1 * p->s);

    msort_with_tmp (p, b1, n1);
    msort_with_tmp (p, b2, n2);

    switch (p->var) {
        case 0: {
            while (n1 > 0 && n2 > 0) {
                if ((*cmp) (b1, b2, arg) <= 0) {
                    *(cuint32 *) tmp = *(cuint32 *) b1;
                    b1 += sizeof (cuint32);
                    --n1;
                }
                else {
                    *(cuint32 *) tmp = *(cuint32 *) b2;
                    b2 += sizeof (cuint32);
                    --n2;
                }
                tmp += sizeof (cuint32);
            }
            break;
        }
        case 1: {
            while (n1 > 0 && n2 > 0) {
                if ((*cmp) (b1, b2, arg) <= 0) {
                    *(cuint64*) tmp = *(cuint64*) b1;
                    b1 += sizeof (cuint64);
                    --n1;
                }
                else {
                    *(cuint64*) tmp = *(cuint64*) b2;
                    b2 += sizeof (cuint64);
                    --n2;
                }
                tmp += sizeof (cuint64);
            }
            break;
        }
        case 2: {
            while (n1 > 0 && n2 > 0) {
                unsigned long *tmpl = (unsigned long *) tmp;
                unsigned long *bl;
                tmp += s;
                if ((*cmp) (b1, b2, arg) <= 0) {
                    bl = (unsigned long *) b1;
                    b1 += s;
                    --n1;
                }
                else {
                    bl = (unsigned long *) b2;
                    b2 += s;
                    --n2;
                }
                while (tmpl < (unsigned long *) tmp) {
                    *tmpl++ = *bl++;
                }
            }
            break;
        }
        case 3: {
            while (n1 > 0 && n2 > 0) {
                if ((*cmp) ((void*) *(const void **) b1, (void*) *(const void **) b2, arg) <= 0) {
                    *(void **) tmp = *(void **) b1;
                    b1 += sizeof (void *);
                    --n1;
                }
                else {
                    *(void **) tmp = *(void **) b2;
                    b2 += sizeof (void *);
                    --n2;
                }
                tmp += sizeof (void *);
            }
            break;
        }
        default: {
            while (n1 > 0 && n2 > 0) {
                if ((*cmp) (b1, b2, arg) <= 0) {
                    memcpy (tmp, b1, s);
                    tmp += s;
                    b1 += s;
                    --n1;
                }
                else {
                    memcpy (tmp, b2, s);
                    tmp += s;
                    b2 += s;
                    --n2;
                }
            }
            break;
        }
    }

    if (n1 > 0) {
        memcpy (tmp, b1, n1 * s);
    }

    memcpy (b, p->t, (n - n2) * s);
}


