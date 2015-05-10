
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ldllist.c
//
// Linearly doubly linked list.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-07-05  Peter S'heeren, Axiris
//
//      * Created.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2014  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "ldllist.h"


VOID  LDLLLinkHeadItem (LDLL_BASE *base, LDLL_ITEM *item)
{
    item->prev = 0;
    item->next = base->first;
    if (base->first) base->first->prev = item; else base->last = item;
    base->first = item;
}


VOID  LDLLLinkTailItem (LDLL_BASE *base, LDLL_ITEM *item)
{
    item->next = 0;
    item->prev = base->last;
    if (base->last) base->last->next = item; else base->first = item;
    base->last = item;
}


LDLL_ITEM  *LDLLUnlinkHeadItem (LDLL_BASE *base)
{
    LDLL_ITEM  *item;
    LDLL_ITEM  *next;

    item = base->first;
    if (item)
    {
        next = item->next;
        base->first = next;
        if (next)
            next->prev = 0;
        else
            base->last = 0;
    }

    return item;
}


LDLL_ITEM  *LDLLUnlinkTailItem (LDLL_BASE *base)
{
    LDLL_ITEM  *item;
    LDLL_ITEM  *prev;

    item = base->last;
    if (item)
    {
        prev = item->prev;
        base->last = prev;
        if (prev)
            prev->next = 0;
        else
            base->first = 0;
    }

    return item;
}


VOID  LDLLUnlinkItem (LDLL_BASE *base, LDLL_ITEM *item)
{
    LDLL_ITEM  *prev;
    LDLL_ITEM  *next;

    prev = item->prev;
    next = item->next;
    if (prev) prev->next = next; else base->first = next;
    if (next) next->prev = prev; else base->last = prev;
}
