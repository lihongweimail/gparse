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


#ifndef GTEXTSTREAM_H
#define GTEXTSTREAM_H


#ifdef __cplusplus
extern "C"
{
#endif


// ----------------------------------------------------------------------------
// Text stream
// The buffered text object allows the tokenizer to tokenize a pre-allocated 
// buffer or from an open file stream.
typedef enum
{
   streamUninitalized,
   streamMemory,
   streamFile,
} gBufferType_e;



typedef struct gTextStream_s
{
   gBufferType_e  type;

   bool           eofflag;

   // Memory buffers.
   // Simple, easy to manage.
   char           *memory;
   const char     *rover;
   int            streamlen;
   bool           owner;

   // File streams
   // A bit trickier. The input from a filestream needs to be buffered to
   // an extent, but not the entire file. The temp buffer starts at 10 chars
   // and expands each time gGetChars is called with a number > than the
   // buffer size.
   char           *fbuffer;
   char           *tempstr;
   int            buffermax;
   // fbufferlen is the length of the string inside the buffer.
   int            fbufferlen;

   // fbufferpos is the position in the filestream the buffer starts
   int            fbufferpos;

   int            fpos;
   FILE           *f;

   // Function pointers for the various stream modes
   char           (*ggetchar)(struct gTextStream_s *);
   char           (*readchar)(struct gTextStream_s *);
   char*          (*readahead)(struct gTextStream_s *, unsigned int);
   void           (*seek)(struct gTextStream_s *, int);
   void           (*freestream)(struct gTextStream_s *);
} gTextStream;


// gStreamFromMemory
// Creates a stream from a memory buffer. If owner is set to true, the memory
// will be freed with the stream is freed.
gTextStream *gStreamFromMemory(char *memory, int length, bool owner);

// gStreamFromFilename
// Opens an archive and loads a skin from it.
gTextStream *gStreamFromFilename(const char *filename);

// gFreeStream
// Frees the given stream. If the stream is a memory buffer, the memory is 
// freed. If the stream is a file, the filestream is clused.
// void gFreeStream(gTextStream *stream);
#define gFreeStream(stream)  stream->freestream(stream)


// gGetChar
// Returns the next character from the given stream. Returns 0 on EOF.
// This function increments the memory/file pointer.
// char gGetChar(gTextStream *stream);
#define gGetChar(stream) stream->ggetchar(stream)


// gReadChar
// Returns the next character from the given stream. Returns 0 on EOF.
// This function does not increment the memory/file pointer.
// char gReadChar(gTextStream *stream);
#define gReadChar(stream) stream->readchar(stream)


// gReadahead
// Returns a pointer to a string containing the next (count) number of 
// characters from the given stream. If the stream is exhausted, the 
// remainder is returned. (returns NULL on EOF).
// This function is a read-ahead function and does not increment 
// the stream pointer.
// char *gReadahead(gTextStream *stream, unsigned int count);
#define gReadahead(stream, count) stream->readahead(stream, count)


// gSeekPos
// Seeks an absolute position within the stream
void gSeekPos(gTextStream *stream, int pos);


// gSeek
// Seeks ahead or behind the given number of characters in the stream.
// void gSeek(gTextStream *stream, int offset);
#define gSeek(stream, offset) stream->seek(stream, offset)


// gStreamEnd
// Returns true of the stream has reached the end of the buffer/file.
bool gStreamEnd(gTextStream *stream);



#endif