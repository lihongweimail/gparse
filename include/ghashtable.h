// Emacs style mode select -*- C++ -*-
// ----------------------------------------------------------------------------
//
// Copyright(C) 2009 Stephen McGranahan
//
// This file is part of gParse
//
// gParse is free software: you can redistribute it and/or modify
// it under the terms of the GNU Limited General Public License as published 
// by the Free Software Foundation, either version 3 of the License, or
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

#ifndef GSTRLIST_H
#define GSTRLIST_H

#include "glist.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// gHashTable
// Generalized hash table which associates an object with a unique string
// identifier. 

typedef struct gHashTable_s gHashTable;


typedef unsigned (*gHashFunc)(const char *string);



// This is the object type that's actually stored in the lists
typedef struct gHashItem_s gHashItem;
typedef struct gHashItem_s
{
   // This is the key associated with the object
   char *key;

   // This is the object 
   void *object;

   // This is the function that should be used to free the object when it is
   // deleted. (This is 'inherited' from the gHashTable object)
   void (*freeFunc)(void *object);

   // Next item in the chain
   gHashItem *next, *prev;
};




// Note: Because all strings would have to be rehashed and objects re-archived 
// if the number of hash bins changed, I'm not going to bother trying to make
// the struct itself support changing the number of bins after the object
// is created.
typedef struct gHashTable_s
{
   // The number of bins in the table
   unsigned int numBins;

   // List of pointers.
   gList *bins;

   // Pointer to the free function to be used on the objects
   // passed to the table.
   void (*freeFunc)(void *object); 

   // Pointer to the hash function to be used on the strings.
   gHashFunc hashFunc;

   // Pointer to the string comparison function
   int (*compFunc)(const char *, const char *);

   // Counts
   unsigned int itemCount;
};



// gNewHashTable
// Creates a new gHashTable object. If freeFunc is null, the code assumes the
// objects in the table should not be freed when they are removed or the table 
// is freed.
gHashTable *gNewHashTable(unsigned numBins, void (*freeFunc)(void *), bool ignoreCase);


// gFreeHashTable
// Frees the given hash table, deleteing all the bins which free all contained
// objects.
void gFreeHashTable(gHashTable *table);


// gAddTableItem
// Attempts to add the given object to the hash table. If the given key already exists
// the function will return false. Otherwise returns true. The string passed will be
// copied. The gHashTable will not free the string you pass to this function.
bool gAddTableItem(gHashTable *table, const char *key, void *item);


// gFindTableItem
// Searches the hash table for the given key and returns a pointer to the associated
// object (returns NULL on fail)
void *gFindTableItem(gHashTable *table, const char *key);


// gRemoveTableItem
// Searches the hash table for the given key and removes it from the bin. If 
// the table is set to free objects, it will be freed. Returns true if the
// key was found, or false if it was not.
bool gRemoveTableItem(gHashTable *table, const char *key);



#ifdef __cplusplus
}
#endif

#endif
