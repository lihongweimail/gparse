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


#ifndef GLIST_H
#define GLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gbool.h"

// ----------------------------------------------------------------------------
// gList
// Resizable array of object pointers. 


typedef struct
{
   void     **list;
   unsigned max;
   unsigned size;

   void     (*freeFunc)(void *);
   bool     freestruct;
} gList;



// gNewList
// Allocates, initializes and returns a new gList object. freeFunc should point
// to the function that should free an array object when it is deleted. If 
// freeFunc is NULL, a NOP is used.
gList *gNewList(void (*freeFunc)(void *));


// gInitList
// Initializes the given list structure. This is called by newlist on the allocated
// list structure.
void gInitList(gList *list, void (*freeFunc)(void *));


// gFreeList
// Frees a gList and all substructures and data contained in it. If the freestruct
// flag is set in the given list struct, the struct itself is freed.
void gFreeList(gList *l);


// gClearList
// Frees all objects contained in a gList, but keeps the list
// structures allocated.
void gClearList(gList *l);


// gAppendListItem
// Adds the given object to the end of the given list, expanding the list if 
// necessary to accommodate the new entry.
void gAppendListItem(gList *l, void *object);


// gInsertListItem
// Inserts the given object into the given list at toindex, moving any 
// objects >= toindex to make room for the new string. This function will also
// expand the list if needed. toindex is clipped to the size of the list.
// so an object won't be placed a index 127129 if the list is 2 items long.
void gInsertListItem(gList *l, void *object, unsigned int toindex);


// gMoveListItem
// Moves an item within the array from index to newindex. If index is outside 
// the bounds of the array, the function will abort. If newindex is outside the
// bounds of the array, the value is clipped to remain within the array.
void gMoveListItem(gList *l, unsigned int index, unsigned int newindex);


// gGetListSize
// returns the size of the list. (that is, the number of entries in the list)
int gGetListSize(gList *l);


// gGetListItem
// Returns the a pointer at index from strlist. If index is out of bounds, NULL 
// is returned.
void *gGetListItem(gList *l, unsigned int index);


// gDeleteListItem
// Deletes an item from the list. If the item index is out of bounds, the
// function simply aborts.
void gDeleteListItem(gList *l, unsigned int index);


// gDeleteListRange
// Deletes a series of items from the list. The indices are clipped to the
// actual dimensions of the list.
void gDeleteListRange(gList *l, unsigned int first, unsigned int last);


// gTrimUnusedList
// This function will reallocate the internal pointer list of the given list
// object to the current list size. This is useful if the list got really big
// and doesn't need to be that big any more.
void gTrimUnusedList(gList *l);


// gMoveListItems
// Moves items from src and appends them to dest. Note: The objects are MOVED which
// means they will not be in the source list anymore. Also Note: This function
// can be dangerous if moving between lists with different free functions.
// Retruns false on error.
bool gMoveListItems(gList *src, unsigned int srcindex, gList *dest, unsigned int count);


#ifdef __cplusplus
}
#endif

#endif

