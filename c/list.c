//
// Created by dingjing on 24-4-9.
//

#include "list.h"


CList* c_list_alloc (void)
{
    CList* list = c_malloc0 (sizeof(CList));

    list->next = NULL;
    list->prev = NULL;

    return list;
}

void c_list_free (CList* list)
{
    CList* i = NULL;
    CList* k = NULL;
    for (i = list; i; i = k) {
        k = i->next;
        c_free(i);
    }
}

void c_list_free_1 (CList* list)
{
    c_free(list);
}

void c_list_free_full (CList* list, CDestroyNotify freeFunc)
{
    c_list_foreach (list, (CFunc) freeFunc, NULL);
    c_list_free (list);
}

CList* c_list_append (CList* list, void* data)
{
    CList* newList = NULL;
    CList* last = NULL;

    newList = c_list_alloc();
    newList->data = data;
    newList->next = NULL;

    if (list) {
        last = c_list_last (list);
        last->next = newList;
        newList->prev = last;
        return list;
    }
    else {
        newList->prev = NULL;
        return newList;
    }
}

CList* c_list_prepend (CList* list, void* data)
{
    CList* newList = NULL;

    newList = c_list_alloc();
    newList->data = data;
    newList->next = list;

    if (list) {
        newList->prev = list->prev;
        if (list->prev) {
            list->prev->next = newList;
        }
        list->prev = newList;
    }
    else {
        newList->prev = NULL;
    }

    return newList;
}

CList* c_list_insert (CList* list, void* data, cint position)
{
    CList* newList;
    CList* tmpList;

    if (position < 0) {
        return c_list_append (list, data);
    }
    else if (position == 0) {
        return c_list_prepend (list, data);
    }

    tmpList = c_list_nth (list, position);
    if (!tmpList) {
        return c_list_append (list, data);
    }

    newList = c_list_alloc();
    newList->data = data;
    newList->prev = tmpList->prev;
    tmpList->prev = tmpList;
    newList->next = tmpList;
    tmpList->prev = newList;

    return list;
}

CList* c_list_insert_sorted (CList* list, void* data, CCompareFunc func)
{

}

CList* c_list_insert_sorted_with_data (CList* list, void* data, CCompareDataFunc func, void* udata)
{

}

CList* c_list_insert_before (CList* list, CList* sibling, void* data)
{
    if (NULL == list) {
        list = c_list_alloc();
        list->data = data;
        c_return_val_if_fail(NULL == sibling, list);
        return list;
    }

    if (NULL != sibling) {
        CList* node = c_list_alloc();
        node->data = data;
        node->prev = sibling->prev;
        node->next = sibling;
        sibling->prev = node;
        if (NULL != node->prev) {
            node->prev->next = node;
            return list;
        }
        else {
            c_return_val_if_fail(sibling == list, node);
            return node;
        }
    }


    
}

CList* c_list_insert_before_link (CList* list, CList* sibling, CList* link)
{
    c_return_val_if_fail(NULL != link, list);
    c_return_val_if_fail(NULL == link->prev, list);
    c_return_val_if_fail(NULL == link->next, list);

    if (NULL == list) {
        c_return_val_if_fail(NULL == sibling, list);
        return link;
    }

    if (NULL != sibling) {
        link->prev = sibling->prev;
        link->next = sibling;
        sibling->prev = link;
        if (NULL != link->prev) {
            link->prev->next = link;
            return list;
        }
        else {
            c_return_val_if_fail(sibling == list, link);
            return link;
        }
    }

    CList* last;

    for (last = list; NULL != last->next; last = last->next);

    last->next = link;
    last->next->prev = last;
    last->next->next = NULL;

    return list;
}

CList* c_list_concat (CList* list1, CList* list2)
{

}

CList* c_list_remove (CList* list, const void* data)
{

}

CList* c_list_remove_all (CList* list, const void* data)
{

}

CList* c_list_remove_link (CList* list, CList* llink)
{

}

CList* c_list_delete_link (CList* list, CList* link)
{

}

CList* c_list_reverse (CList* list)
{

}

CList* c_list_copy (CList* list)
{

}

CList* c_list_copy_deep (CList* list, CCopyFunc func, void* udata)
{

}

CList* c_list_nth (CList* list, cuint n)
{

}

CList* c_list_nth_prev (CList* list, cuint n)
{

}

CList* c_list_find (CList* list, const void* data)
{

}

CList* c_list_find_custom (CList* list, const void* data, CCompareFunc func)
{

}

cint c_list_position (CList* list, CList* llink)
{

}

cint c_list_index (CList* list, const void* data)
{

}

CList* c_list_last (CList* list)
{

}

CList* c_list_first (CList* list)
{

}

cuint c_list_length (CList* list)
{

}

void c_list_foreach (CList* list, CFunc func, void* u_data)
{

}

CList* c_list_sort (CList* list, CCompareFunc compareFunc)
{

}

CList* c_list_sort_with_data (CList* list, CCompareDataFunc  compareFunc, void* udata)
{

}

void* c_list_nth_data (CList* list, cuint n)
{

}
