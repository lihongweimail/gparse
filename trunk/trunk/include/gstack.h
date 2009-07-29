// Emacs style mode select -*- C++ -*-
// ----------------------------------------------------------------------------
//
// Copyright(C) 2009 Stephen McGranahan
//
// This file is part of gParse
//
// gParse is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// ----------------------------------------------------------------------------
//
// gParse is a text file parsing system which operates with a few basic
// behaviors predefined, and others optionalized. This creates a token 
// generator that is at once very simple to use and works well for most
// situations.
//
// ----------------------------------------------------------------------------


#ifndef GSTACK_H
#define GSTACK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gbool.h"

// ----------------------------------------------------------------------------
// gStack
// Generalized stack container object.


typedef struct gStackItem_s gStackItem;
typedef struct gStackItem_s
{
   void *item;
   gStackItem *prev;
};

typedef struct
{
   gStackItem *top, *unused;

   void     (*freeFunc)(void *);
   bool     freestruct;
} gStack;



// gNewStack
// Allocates, initializes and returns a new gStack object. freeFunc should point
// to the function that should free an array object when it is deleted. If 
// freeFunc is NULL, gFreeNOP is used. 
gStack *gNewStack(void (*freeFunc)(void *));


// gInitStack
// Initializes the given list structure. This is called by gNewStack on the 
// allocated stack structure.
void gInitStack(gStack *stack, void (*freeFunc)(void *));


// gFreeStack
// Frees a gStack and all substructures and data contained in it. If the 
// freestructflag is set in the given list struct, the struct itself is freed.
void gFreeStack(gStack *s);


// gClearStack
// Frees all objects contained in a gStack, but keeps the list
// structures allocated.
void gClearStack(gStack *s);


// gPushObject
// Adds the given object to the end of the given stack.
void gPushObject(gStack *s, void *object);


// gGetStackTop
// Returns the current top item on the stack
void *gGetStackTop(gStack *s);


// gGetStackSize
// Polls the stack and returns the number of entries in the stack.
int gGetStackSize(gStack *s);


// gPopStack
// Deletes the top of the stack and returns the new top item (returns NULL if
// the stack is empty).
void *gPopStack(gStack *s);


#ifdef __cplusplus
}
#endif

#endif

