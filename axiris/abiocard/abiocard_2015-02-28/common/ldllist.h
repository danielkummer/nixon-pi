
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ldllist.h
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


#ifndef __LDLLIST_H__
#define __LDLLIST_H__


#include "platform.h"


typedef struct  _LDLL_ITEM  LDLL_ITEM;

struct  _LDLL_ITEM
{
    LDLL_ITEM  *prev;
    LDLL_ITEM  *next;
};


typedef struct  _LDLL_BASE  LDLL_BASE;

struct  _LDLL_BASE
{
    LDLL_ITEM  *first;
    LDLL_ITEM  *last;
};


VOID  LDLLLinkHeadItem (LDLL_BASE *base, LDLL_ITEM *item);

VOID  LDLLLinkTailItem (LDLL_BASE *base, LDLL_ITEM *item);

LDLL_ITEM  *LDLLUnlinkHeadItem (LDLL_BASE *base);

LDLL_ITEM  *LDLLUnlinkTailItem (LDLL_BASE *base);

VOID  LDLLUnlinkItem (LDLL_BASE *base, LDLL_ITEM *item);


static
LDLL_ITEM  __inline  *LDLLGetHeadItem (LDLL_BASE *base)
{
    return base->first;
}


static
LDLL_ITEM  __inline  *LDLLGetTailItem (LDLL_BASE *base)
{
    return base->last;
}


static
LDLL_ITEM  __inline  *LDLLGetPrevItem (LDLL_ITEM *item)
{
    return item->prev;
}


static
LDLL_ITEM  __inline  *LDLLGetNextItem (LDLL_ITEM *item)
{
    return item->next;
}


#endif  // __LDLLIST_H__
