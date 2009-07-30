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

/** \file ghashtable.h
    \brief Generalized hashtable.

    Contains definitions and functions for gHashTable, a generalized hash table
    which archives, and searches objects by a string "key".
 */


#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// gHashTable
// Generalized hash table which associates an object with a unique string
// identifier. 


/** \struct gHashTable
 *  \brief Generalized hash table object.
 *
 *  gHashTable archives, and retrieves objects based on a string "key".
 *  Only one occurance of each key is allowed in the table, and the table can 
 *  optionally ignore case. These tables can also optionally be used for memory
 *  management of the contents. It is recommended that these tables be created 
 *  with gNewHashTable and freed with gFreeHashTable.
 *  \see ghashtable.h::gNewHashTable, ghashtable.h::gFreeHashTable, 
 *       ghashtable.h::gFreeHashTable, ghashtable.h::gAddTableItem, 
 *       ghashtable.h::gFindTableItem, ghashtable.h::gRemoveTableItem,
 *       ghashtable.h::gRehashTable
 */
typedef struct gHashTable
{
   gList *chains;  //!< List of hash chains in the table.

   /** Function called when objects are removed from the table. Can be a NOP
       function. */
   void (*freeFunc)(void *object);

   /** Function used to hash the strings passed to the table. */
   unsigned (*hashFunc)(const char *string);

   /** Function used to compare individual elements in a hash chain. */
   int (*compFunc)(const char *s1, const char *s2);

   /** Number of items in the table. */
   unsigned int itemCount;
} gHashTable;




/** \fn gHashTable *gNewHashTable(unsigned numChains, void (*freeFunc)(void *), bool ignoreCase)
 *  \brief Creates a new hash table.
 *
 *  Creates a new gHashTable object with the given number of hash chains.
 *  If freeFunc is null, the code assumes the objects in the table should 
 *  not be freed when they are removed or the table is freed.
 *
 *  @param[in] numChains Initian number of chains in the table.
 *  @param[in] freeFunc Function used to free items when they are removed. Can be NULL.
 *  @param[in] ignoreCase If true, the hash table will ignore case when storing/finding items.
 *  @returns Newly created hash table or NULL on error.
 */
gHashTable *gNewHashTable(unsigned numChains, void (*freeFunc)(void *), bool ignoreCase);


/** \fn void gFreeHashTable(gHashTable *table)
 *  \brief Frees a hash table.
 *
 *  Frees the given hash table, calling the freeFunc for all objects in the table.
 *
 *  @param[in] table Hash table to free.
 */
void gFreeHashTable(gHashTable *table);


/** \fn bool gAddTableItem(gHashTable *table, const char *key, void *item)
 *  \brief Adds an item to a hash table.
 *
 *  Attempts to add the given item to the hash table. If the given key already exists
 *  the function will return false. Otherwise returns true. The string passed will be
 *  copied. The gHashTable will not free the string you pass to this function.
 *
 *  @param[in] table Table to add the item to.
 *  @param[in] key String key to associate with the item
 *  @param[in] item Actual item to be stored. 
 *  @returns true on success, false if the item already exists.
 */
bool gAddTableItem(gHashTable *table, const char *key, void *item);


/** \fn void *gFindTableItem(gHashTable *table, const char *key)
 *  \brief Searches a hash table for an item with a matching key.
 *
 *  Searches the hash table for the given key and returns a pointer to the associated
 *  item.
 *
 *  @param[in] table Hash table to search
 *  @param[in] key Hash key to search for
 *  @returns Associated object on success, NULL on fail.
 */
void *gFindTableItem(gHashTable *table, const char *key);


/** \fn bool gRemoveTableItem(gHashTable *table, const char *key)
 *  \brief Removes an item from a hash table.
 *
 *  Searches the hash table for the given key and removes it from the bin. If 
 *  the table is set to free objects, it will be freed. 
 *
 *  @param[in] table Table to remove the item from
 *  @param[in] key The key of the item to be removed.
 *  @return True if the item was successfully removed, false if it could not be found.
 */
bool gRemoveTableItem(gHashTable *table, const char *key);


/** \fn bool gRehashTable(gHashTable *table, unsigned int numChains)
 *  \brief Changes the number of hash chains
 *
 *  Resizes and re-hashes the given table to the specified number of chains.
 *
 *  @param[in] table Hash table to be rehashed
 *  @param[in] numChains Number of chains the table should have.
 *  @returns true on success, false on error.
 */
bool gRehashTable(gHashTable *table, unsigned int numChains);



#ifdef __cplusplus
}
#endif

#endif
