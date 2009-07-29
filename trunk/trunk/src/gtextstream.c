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

#include "gtokenize.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ----------------------------------------------------------------------------
// Text stream.
// The buffered text object allows the tokenizer to tokenize a pre-allocated 
// buffer or from an open file stream.

static gTextStream *newStream()
{
   gTextStream *ret = (gTextStream *)malloc(sizeof(gTextStream));
   memset(ret, 0, sizeof(*ret));
   return ret;
}


// ----------------------------------------------------------------------------
// File streams
// File streams are fairly complex little things that offer, among other things,
// buffered input of files. This makes accessing text from a file look similar
// to accessing text from memory.


// "Moves" a buffer based on the integer offset provided. If the offset is
// negative, the stream moves backwards. This function clips the movement to
// the first and last characters of the text file. After the offset, the code
// will shift the data in the buffer, and clear the newly exposed data.
void shiftFileBuffer(gTextStream *stream, int offset)
{
   char        *buffer = stream->fbuffer; 
   int         size = stream->fbufferlen;
   int         i;


   if(stream->fbufferpos + offset < 0)
      offset = -stream->fbufferpos;

   stream->fbufferpos += offset;

   if(stream->fbufferpos >= stream->streamlen)
   {
      stream->fbuffer[0] = 0;
      stream->fbufferlen = 0;
      stream->fbufferpos = stream->streamlen;
      stream->eofflag = true;
      return;
   }

   if(stream->fbufferpos + size > stream->streamlen)
      stream->fbufferlen = stream->streamlen - stream->fbufferpos;

   if(offset == 0)
      return;
   if(abs(offset) >= size)
   {
      memset(buffer, 0, size);
      return;
   }

   if(offset > 0)
   {
      for(i = 0; i < size - offset; i++)
         buffer[i] = buffer[i + offset];

      for(; i < size; i++)
         buffer[i] = 0;
   }
   else
   {
      for(i = size - 1; i + offset >= 0; i--)
         buffer[i] = buffer[i + offset];

      for(; i >= 0; i--)
         buffer[i] = ' ';
   }
}



// This acts as a wrapper function to maintain consistency between the file
// pointer and the data contained in the struct. It is only used by internal
// functions.
static void fileSeek(gTextStream *stream, int offset, int type)
{
   int position = stream->fpos;

   if(type == SEEK_SET)
      position = offset;
   else if(type == SEEK_CUR)
      position += offset;
   else if(type == SEEK_END)
      position = stream->streamlen + offset;
   else
      return;

   if(position < 0)
   {
      position = 0;
      stream->eofflag = false;
   }
   else if(position > stream->streamlen)
   {
      position = stream->streamlen;
      stream->eofflag = true;
   }
   else
      stream->eofflag = false;

   fseek(stream->f, position, SEEK_SET);
   stream->fpos = position;
}


// Acts as a wrapper for fread to maintain consistency between the FILE 
// pointer and internal data.
static int fileRead(void *data, int size, int count, gTextStream *stream)
{
   int ret;

   if(stream->eofflag)
      return 0;

   ret = fread(data, size, count, stream->f);
   stream->fpos += ret;
   return ret;
}



// This function returns the next character in the stream and 'pushes' the 
// input buffer ahead one character. Returns 0 on EOF
static char getCharFile(gTextStream *stream)
{
   char ret = 0;

   if(stream->eofflag)
      return 0;

   ret = stream->fbuffer[0];
   shiftFileBuffer(stream, 1);
   
   if(stream->fpos < stream->streamlen)
      fileRead(&stream->fbuffer[stream->fbufferlen - 1], 1, 1, stream);

   // An empty buffer means EOF
   if(stream->fbufferlen == 0)
   {
      stream->eofflag = true;
      return 0;
   }

   return ret;
}



// Returns the next character in the stream, but does not move the buffer.
static char readCharFile(gTextStream *stream)
{
   char ret = 0;

   if(stream->eofflag)
      return 0;

   ret = stream->fbuffer[0];

   if(stream->fbufferlen == 0)
      stream->eofflag = true;

   return ret;
}


// Attempts to read the next (count) characters into the file buffer. The 
// amount returned may be cut short if the end of the stream is encountered.
// A pointer to the null-terminated tempstring member is returned on success
// or NULL on EOF.
static char *readAheadFile(gTextStream *stream, unsigned int count)
{
   int toread;

   if(stream->eofflag)
      return NULL;

   // Check for EOF
   if(stream->fbufferpos >= stream->streamlen)
   {
      stream->fbufferlen = 0;
      stream->fbuffer[0] = 0;

      stream->eofflag = true;
      return NULL;
   }

   // Make sure the temp buffers are big enough for the requested data.
   if(stream->buffermax <= (int)count)
   {
      // read-ahead has exceeded buffer length, so increase buffer size.
      int newsize = count + 1;

      stream->fbuffer = (char *)realloc(stream->fbuffer, newsize);
      stream->tempstr = (char *)realloc(stream->tempstr, newsize);
      memset(stream->fbuffer + stream->buffermax, 0, newsize - stream->buffermax); 
      stream->buffermax = newsize;
   }

   // The characters needed might already be buffered. 
   toread = count - stream->fbufferlen;

   if(toread > 0)
   {
      // TODO: This should REALLY be done prior to the buffer re-allocation.
      // If the function is called requesting MAX_UINT characters, the function
      // will currently try to allocate that amount even if the file only has
      // one character left!!
      if(stream->fpos + toread >= stream->streamlen)
         toread = stream->streamlen - stream->fpos;

      fileRead(stream->fbuffer + stream->fbufferlen, 1, toread, stream);
      stream->fbuffer[stream->buffermax - 1] = 0;
   }

   if(stream->fbufferlen == 0)
   {
      stream->eofflag = true;
      return NULL;
   }

   // tempstr is returned because it is NULL-terminated.
   strncpy(stream->tempstr, stream->fbuffer, count);
   stream->tempstr[count] = 0;

   return stream->tempstr;
}




static void seekFile(gTextStream *stream, int offset)
{
   int pos = stream->fbufferpos;
   int len;

   if(offset == 0 || (offset > 0 && stream->eofflag))
      return;

   shiftFileBuffer(stream, offset);

   if(offset < 0)
      offset = pos - stream->fbufferpos;
   else
      offset = stream->fbufferpos - pos;

   pos = stream->fbufferpos;

   if(pos >= stream->streamlen)
   {
      fileSeek(stream, 1, SEEK_END);
      stream->fbufferpos = stream->streamlen;
      stream->fbufferlen = 0;
      stream->fbuffer[0] = 0;
      stream->eofflag = true;
   }
   else
   {
      if(abs(offset) >= stream->fbufferlen)
      {
         int clamped;
         len = stream->buffermax - 1;
         if(stream->fbufferpos + len > stream->streamlen)
            len = stream->streamlen - stream->fbufferpos;

         if(stream->fpos != stream->fbufferpos)
            fileSeek(stream, stream->fbufferpos, SEEK_SET);
         
         if(!stream->eofflag)
         {
            clamped = fileRead(stream->fbuffer, 1, len, stream);

            stream->fbufferlen = clamped;
            stream->fbuffer[clamped] = 0;
         }

         if(stream->fbufferlen == 0)
            stream->eofflag = true;
      }
      else
      {
         if(offset < 0)
         {
            int readlen = -offset;

            if(stream->fpos != stream->fbufferpos)
               fileSeek(stream, stream->fbufferpos, SEEK_SET);

            fileRead(stream->fbuffer, 1, readlen, stream);
         }
         else
         {
            int keeplen = stream->fbufferlen - offset;
            int readlen = stream->buffermax - 1 - keeplen;
            int clamped;

            if(stream->fpos != stream->fbufferpos + keeplen)
               fileSeek(stream, stream->fbufferpos + keeplen, SEEK_SET);

            if(stream->fpos < stream->streamlen)
               clamped = fileRead(stream->fbuffer + keeplen, 1, readlen, stream);
            else
               clamped = 0;

            stream->fbufferlen = keeplen + clamped;

            if(stream->fbufferlen == 0)
               stream->eofflag = true;
         }

         stream->fbuffer[stream->fbufferlen] = 0;
      }
   }
}



static void freeStreamFile(gTextStream *stream)
{
   if(stream->fbuffer)
      free(stream->fbuffer);

   if(stream->tempstr)
      free(stream->tempstr);

   fclose(stream->f);

   free(stream);
}




gTextStream *gStreamFromFile(FILE *file)
{
   gTextStream *ret = newStream();

   ret->f = file;
   if(ret->f)
      ret->type = streamFile;
   else
   {
      gFreeStream(ret);
      return NULL;
   }

   fseek(file, 0, SEEK_END);
   ret->streamlen = ftell(file);
   fseek(file, 0, SEEK_SET);

   ret->buffermax = 10;
   ret->fbuffer = malloc(sizeof(char) * ret->buffermax);
   ret->tempstr = malloc(sizeof(char) * ret->buffermax);
   ret->tempstr[0] = 0;

   if(ret->streamlen < 9)
      ret->fbufferlen = fread(ret->fbuffer, 1, ret->streamlen, file);
   else
      ret->fbufferlen = fread(ret->fbuffer, 1, 9, file);

   ret->fbuffer[ret->fbufferlen] = 0;

   ret->fbufferpos = 0;
   ret->fpos = ftell(file);

   ret->ggetchar = getCharFile;
   ret->readchar = readCharFile;
   ret->seek = seekFile;
   ret->readahead = readAheadFile;
   ret->freestream = freeStreamFile;

   return ret;
}


gTextStream *gStreamFromFilename(const char *filename)
{
   FILE *f = fopen(filename, "rb");

   if(!f)
      return NULL;

   return gStreamFromFile(f);
}




// ----------------------------------------------------------------------------
// Memory streams

static char getCharMemory(gTextStream *stream)
{
   char ret = 0;

   if(stream->eofflag)
      return 0;

   ret = *stream->rover;
   stream->rover ++;

   if(stream->rover - stream->memory >= stream->streamlen)
      stream->eofflag = true;

   return ret;
}


static char readCharMemory(gTextStream *stream)
{
   char ret = 0;

   if(stream->eofflag)
      return 0;

   if(stream->rover - stream->memory >= stream->streamlen)
      stream->eofflag = true;
   else
      ret = *stream->rover;

   return ret;
}


static char *readAheadMemory(gTextStream *stream, unsigned int count)
{
   if(stream->eofflag)
      return NULL;

   if(stream->buffermax <= (int)count)
   {
      int newsize = count + 1;
      stream->tempstr = (char *)realloc(stream->tempstr, newsize);
      memset(stream->tempstr, 0, newsize);
      stream->buffermax = newsize;
   }
   
   if((stream->rover - stream->memory) >= stream->streamlen)
   {
      stream->eofflag = true;
      return NULL;
   }

   if((stream->rover - stream->memory) + (int)count >= stream->streamlen)
      count = stream->streamlen - (stream->rover - stream->memory);

   strncpy(stream->tempstr, stream->rover, count);
   stream->tempstr[count] = 0;

   return stream->tempstr;
}


static void seekMemory(gTextStream *stream, int offset)
{
   const char *base = stream->memory, *marker = stream->rover;

   if(offset == 0 || (offset > 0 && stream->eofflag))
      return;

   if(offset < 0 && marker + offset < base)
   {
      stream->rover = stream->memory;
      stream->eofflag = stream->streamlen > 0 ? false : true;
   }
   else if(marker + offset >= base + stream->streamlen)
   {
      stream->rover = stream->memory + stream->streamlen;
      stream->eofflag = true;
   }
   else
   {
      stream->rover += offset;
      stream->eofflag = false;
   }
}


static void freeStreamMemory(gTextStream *stream)
{
   if(stream->owner)
      free(stream->memory);

   free(stream);
}



gTextStream *gStreamFromMemory(char *memory, int length, bool owner)
{
   gTextStream *ret = newStream();

   ret->type = streamMemory;

   ret->memory = memory;
   ret->rover = memory;
   ret->streamlen = length;
   ret->owner = owner;

   ret->buffermax = 10;
   ret->tempstr = malloc(sizeof(char) * ret->buffermax);
   ret->tempstr[0] = 0;

   ret->ggetchar = getCharMemory;
   ret->readchar = readCharMemory;
   ret->seek = seekMemory;
   ret->readahead = readAheadMemory;
   ret->freestream = freeStreamMemory;

   return ret;
}



// ----------------------------------------------------------------------------
// Generic stream functions.

// gSeekPos
// Seeks an absolute position within the stream
void gSeekPos(gTextStream *stream, int pos)
{
   int offset;

   if(pos < 0)
      pos = 0;
   if(pos >= stream->streamlen)
      pos = stream->streamlen;

   offset = pos - stream->fbufferpos;

   gSeek(stream, offset);
}


// gStreamEnd
// Returns true of the stream has reached the end of the buffer/file.
bool gStreamEnd(gTextStream *stream)
{
   return stream->eofflag;
}
