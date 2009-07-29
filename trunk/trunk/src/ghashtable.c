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


void hashFreeNOP(void *obj) {}

// Used to free the entry objects.
void freeHashEntry(gHashItem *item)
{
   if(!item)
      return;

   item->freeFunc(item->object);
   free(item->key);
   free(item);
}


unsigned calcHashKey(const char *string)
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



unsigned calcHashKeyS(const char *string)
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


static gHashItem *makeBinHead()
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
gHashTable *gNewHashTable(unsigned numBins, void (*freeFunc)(void *), bool ignoreCase)
{
   gHashTable *ret;
   unsigned    i;

   if(!numBins)
      return NULL;

   ret = malloc(sizeof(gHashTable));
   memset(ret, 0, sizeof(*ret));

   ret->numBins = numBins;
   ret->bins = gNewList(free);

   if(freeFunc == NULL)
      ret->freeFunc = hashFreeNOP;
   else
      ret->freeFunc = freeFunc;

   for(i = 0; i < numBins; i++)
      gAppendListItem(ret->bins, makeBinHead());

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


   for(i = 0; i < table->numBins; i++)
   {
      head = (gHashItem *)gGetListItem(table->bins, i);

      for(it = head->next; it != head; it = next)
      {
         next = it->next;
         freeHashEntry(it);
         it = next; 
      }
   }

   gFreeList(table->bins);
   free(table);
}



// gRehashTable
// Resizes and re-hashes the given table to the specified number of bins
bool gRehashTable(gHashTable *table, unsigned int numBins)
{
   unsigned int i;
   gHashItem    *list = NULL, *it, *next;

   // check everything out
   if(!table || !numBins || !table->numBins || numBins == table->numBins)
      return false;

   // Gather all the hash items
   for(i = 0; i < table->bins->size; i++)
   {
      gHashItem *head = (gHashItem *)gGetListItem(table->bins, i);
      gHashItem *first, *last;

      if(head->next != head)
      {
         first = head->next;
         last = head->prev;

         head->next = head->prev = head;
         first->prev = last->next = NULL;
      }
      else
         first = last = NULL;

      if(!list)
         list = first;
      else if(first && last)
      {
         (last->next = list)->prev = last;
         list = first;
      }
   }

   while(table->bins->size < numBins)
      gAppendListItem(table->bins, makeBinHead());
   if(table->bins->size > numBins)      
      gDeleteListRange(table->bins, numBins, table->bins->size - 1);

   table->numBins = table->bins->size;

   for(it = list; it; it = next)
   {
      unsigned int index = table->hashFunc(it->key) % table->numBins;
      gHashItem *head;
      
      next = it->next;
      head = (gHashItem *)gGetListItem(table->bins, index);
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

   index = table->hashFunc(key) % table->numBins;
   foundHead = head = (gHashItem *)gGetListItem(table->bins, index);

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



