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

#include "ghashtable.h"
#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
// gHashTable
// Generalized hash table which associates an object with a unique string
// identifier. 


// SoM: This is only ever used in this file
// This is the object type that's actually stored in the lists
typedef struct gHashItem
{
   // This is the key associated with the object
   char *key;

   // This is the object 
   void *object;

   // This is the function that should be used to free the object when it is
   // deleted. (This is 'inherited' from the gHashTable object)
   void (*freeFunc)(void *object);

   // Next item in the chain
   struct gHashItem *next, *prev;
} gHashItem;



void hashFreeNOP(void *obj) {}

// Used to free the entry objects.
static void freeHashEntry(gHashItem *item)
{
   if(!item)
      return;

   item->freeFunc(item->object);
   free(item->key);
   free(item);
}


static unsigned calcHashKey(const char *string)
{
   const char *c = string;
   unsigned h = 0;

   if(!string)
      return 0;

   while(*c)
   {
      h = 5 * h + toupper(*c);
      ++c;
   }

   return h;
}



static unsigned calcHashKeyS(const char *string)
{
   const char *c = string;
   unsigned h = 0;

   if(!string)
      return 0;

   while(*c)
   {
      h = 5 * h + *c;
      ++c;
   }

   return h;
}


static gHashItem *makeChainHead()
{
   gHashItem *it = malloc(sizeof(gHashItem));
   memset(it, 0, sizeof(*it));
   it->next = it->prev = it;
   return it;
}




// gNewHashTable
// Creates a new gHashTable object. If freeFunc is null, the code assumes the
// objects in the table should not be freed when they are removed or the table 
// is freed.
gHashTable *gNewHashTable(unsigned numChains, void (*freeFunc)(void *), bool ignoreCase)
{
   gHashTable *ret;
   unsigned    i;

   if(!numChains)
      return NULL;

   ret = malloc(sizeof(gHashTable));
   memset(ret, 0, sizeof(*ret));

   ret->chains = gNewList(free);

   if(freeFunc == NULL)
      ret->freeFunc = hashFreeNOP;
   else
      ret->freeFunc = freeFunc;

   for(i = 0; i < numChains; i++)
      gAppendListItem(ret->chains, makeChainHead());

   if(ignoreCase)
   {
      ret->hashFunc = calcHashKey;
      ret->compFunc = stricmp;
   }
   else
   {
      ret->hashFunc = calcHashKeyS;
      ret->compFunc = strcmp;
   }

   return ret;
}


// gFreeHashTable
// Frees the given hash table, deleteing all the bins which free all contained
// objects.
void gFreeHashTable(gHashTable *table)
{
   unsigned i;
   gHashItem *head, *it, *next;

   if(!table)
      return;


   for(i = 0; i < table->chains->size; i++)
   {
      head = (gHashItem *)gGetListItem(table->chains, i);

      for(it = head->next; it != head; it = next)
      {
         next = it->next;
         freeHashEntry(it);
         it = next; 
      }
   }

   gFreeList(table->chains);
   free(table);
}



// gRehashTable
// Resizes and re-hashes the given table to the specified number of bins
bool gRehashTable(gHashTable *table, unsigned int numChains)
{
   unsigned int i;
   gHashItem    *list = NULL, *it, *next;

   // check everything out
   if(!table || !numChains || !table->chains->size || 
      numChains == table->chains->size)
      return false;

   // Gather all the hash items
   for(i = 0; i < table->chains->size; i++)
   {
      gHashItem *head = (gHashItem *)gGetListItem(table->chains, i);
      gHashItem *first, *last;

      // If there are items in the chain, add it to the gathering chain.
      if(head->next != head)
      {
         first = head->next;
         last = head->prev;

         head->next = head->prev = head;
         first->prev = last->next = NULL;
      }
      else
         first = last = NULL;

      if(!list && first)
         list = first;
      else if(first && last)
      {
         (last->next = list)->prev = last;
         list = first;
      }
   }

   // Expand the table if the new size is larger.
   while(table->chains->size < numChains)
      gAppendListItem(table->chains, makeChainHead());

   // Delete extra chains if the new size is smaller.
   if(table->chains->size > numChains)      
      gDeleteListRange(table->chains, numChains, table->chains->size - 1);

   // Now, run through the collector chain and hash items it contains.
   for(it = list; it; it = next)
   {
      unsigned int index = table->hashFunc(it->key) % table->chains->size;
      gHashItem *head;
      
      next = it->next;
      head = (gHashItem *)gGetListItem(table->chains, index);
      if(!head)
      {
         // Error out?
         continue;
      }

      (it->next = head->next)->prev = it;
      (it->prev = head)->next = it;
   }

   return true;
}




// This is a bit hackish but it works. This is used by another function to get
// the bin found by findHashItem so the key function doesn't have to be called
// twice.
static gHashItem *foundHead = NULL;

static gHashItem *findHashItem(gHashTable *table, const char *key)
{
   gHashItem *item, *head;
   unsigned int index;

   if(!table || !key)
      return NULL;

   index = table->hashFunc(key) % table->chains->size;
   foundHead = head = (gHashItem *)gGetListItem(table->chains, index);

   for(item = head->next; item != head; item = item->next)
   {
      if(!table->compFunc(item->key, key))
         return item;
   }

   return NULL;
}



// gAddTableItem
// Attempts to add the given object to the hash table. If the given key already exists
// the function will return false. Otherwise returns true. The string passed will be
// copied. The gHashTable will not free the string you pass to this function.
bool gAddTableItem(gHashTable *table, const char *key, void *item)
{
   gHashItem *it;

   if(!table || !key || !item)
      return false;

   if(findHashItem(table, key))
      return false;

   it = malloc(sizeof(gHashItem));
   memset(it, 0, sizeof(*it));

   it->freeFunc = table->freeFunc;
   it->key = strdup(key);
   it->object = item;

   (it->next = foundHead->next)->prev = it;
   (it->prev = foundHead)->next = it;

   table->itemCount++;

   return true;
}


// gFindTableItem
// Searches the hash table for the given key and returns a pointer to the associated
// object (returns NULL on fail)
void *gFindTableItem(gHashTable *table, const char *key)
{
   gHashItem *item = findHashItem(table, key);

   return item ? item->object : NULL;
}


// gRemoveTableItem
// Searches the hash table for the given key and removes it from the bin. If 
// the table is set to free objects, it will be freed. Returns true if the
// key was found, or false if it was not.
bool gRemoveTableItem(gHashTable *table, const char *key)
{
   gHashItem *it = findHashItem(table, key);

   if(it != NULL)
   {
      (it->next->prev = it->prev)->next = it->next;

      freeHashEntry(it);
      table->itemCount--;
      return true;
   }

   return false;
}



