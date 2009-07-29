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

#ifndef GTOKENIZE_H
#define GTOKENIZE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "qstring.h"
#include "glist.h"
#include "ghashtable.h"
#include "gtextstream.h"





// ----------------------------------------------------------------------------
// gTokenParms
// Holds information used by the tokenizer engine to determine how to tokenize 
// the input stream.


typedef struct
{
   char *token;      // original char token
   int  newtype;     // new type
   char *newtoken;   // new token (optional)
} gKeyword;


typedef struct
{
   char token[4];       // String (max 3 chars + 0x0)
   int  newtype;        // new type
} gSymbol;



typedef struct
{
   char escchar;
   char replacechar;
} gEscape;



typedef enum
{
   gIgnoreCase = 0x1,      // Ignore case when marking keywords
   gNewlineTokens = 0x2,   // Add a newline token when a newline char is encountered
   gIgnoreUnknowns = 0x4,  // Ignore chars that are unknown or unparsable.
   gIgnoreEscapes = 0x8,   // Ignore escape sequences in string literals eg. \n,
} gParseFlags_e;


// Error function
typedef void (*gErrorFunc)(const char *, ...);
typedef const char *(*gGetErrorFunc)(void);
typedef int  (*strCompFunc)(const char *, const char *, unsigned int);


typedef struct 
{
   char       nullchar;
   gKeyword   *keywlist;    // list of keywords used in tokenizing
   gSymbol    *symbollist;  // list of symbols used in tokenizing
   gEscape    *escapelist;  // List of escape sequence chars in string literals

   // Comments:
   // The tokenizer supports up to two comment styles. Each specified by the 
   // strings. commentXs is the marker denoting the beginning of a comment
   // (eg. // or /* or ') and commentXe is the marker denoting the end
   // (eg. '\n' or */)
   char       *comment1s, *comment1e;
   char       *comment2s, *comment2e;
   // These strings are never modified so it's safe to assign string
   // constants to them.
   // The default values are as such:
   // comment1s: "//"
   // comment1e: "\n"
   // comment2s: "/*"
   // comment2e: "*/"

   // Parser flags (See gParseFlags_e)
   int        flags;

   // Function used to report errors in the tokenizing process.
   gErrorFunc setError;
   gGetErrorFunc getError;

   // This function is set internally.
   strCompFunc    strncmp;
} gTokenParms;



// gNewTokenParms
// Allocates and initializes and then returns a new gTokenParms structure.
gTokenParms *gNewParms();


// gFreeTokenParms
// Frees an allocated gTokenParms structure. Currently this just calls free
// but it is recommended this function be used.
void gFreeParms(gTokenParms *parameters);




// ----------------------------------------------------------------------------
// gToken
// This is the struct that holds a token

typedef enum
{
   tUnknown,
   tIdentifier,      // Starts with a letter or _ and contains A-Z, a-z, 0-9, and _
   tInteger,         // Starts with a number and contains only 0-9
   tHexInt,          // Starts with 0x and contains 0-9, a-f, and A-F
   tDecimal,         // Starts with either . or 0-9 and contains one . 
                     // and optionally IEEE formatting stuff...
   tString,          // A quoted string

   tLineBreak,       // end of a line

   tEOF,             // End of file token


   tLastBaseToken,   // First custom token type
} gTokenType_e;



typedef struct gToken_s
{
   int          type;         // Type holds the gTokenType_e or user value
   char         *token;       // The actual string which comprises the token
   int          linenum;      // Line number the token occurs on
   int          charnum;      // Char number the token occurs on (within the line)
} gToken;


// gCreateToken
// Creates and returns a single token struct from the given parameters.
gToken *gCreateToken(const char *token, int type, int linenum, int charnum);

// gFreeToken
// Frees a single token. Changed for use with gList
void gFreeToken(void *object);






// ----------------------------------------------------------------------------
// gTokenStream
// This object is the means by which the tokenizer actually does most of the 
// work.
typedef struct gTokenStream_s
{
   gTokenParms *parameters;
   gTextStream *stream;

   // Temporary buffer used by the tokenizing functions
   qstring_t   *tokenbuf;

   // Name of the file or stream the tokens are from.
   char        *name;

   // The current linenumber and char number
   int         linenum, charnum; 

   // Set to true when the stream is exhausted. This is set when gGetNextToken 
   // reaches the end of the stream.
   bool        endofstream;
   
   // Currently cached tokens
   gList       *tcache;
   int         cfirst, clast;
} gTokenStream;



// gCreateTokenStream
// Creates a new tokenstream from the given parameters.
gTokenStream *gCreateTokenStream(gTokenParms *parameters, gTextStream *stream, const char *name);


// gFreeTokenStream
// Frees a tokenstream made with gCreateTokenStream
void gFreeTokenStream(gTokenStream *tstream);


// gGetNextToken
// Returns a pointer to the next token in the stream.
gToken *gGetNextToken(gTokenStream *tstream);

// gGetToken
// Returns the token at the given index. Index must be >= the first token in the
// streams cache--that is, >= strea->cfirst. If index > stream->clast, 
// the cache of the stream is read-ahead to index. Returns NULL if index was less
// than cfirst. If not enough tokens could be parsed from the stream to get to
// index, the last token in the stream is returned (tEOF)
gToken *gGetToken(gTokenStream *tstream, int index);

// gClearTCache
// Clears the token cache within the stream. All but the very last token cached 
// are freed.
void gClearTCache(gTokenStream *tstream);


#include "gtpattern.h"


#ifdef __cplusplus
}
#endif

#endif //GTOKENIZE_H


