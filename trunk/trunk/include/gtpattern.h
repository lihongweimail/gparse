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

#ifndef GTPATTERN_H
#define GTPATTERN_H

#include "gstack.h"

#ifdef __cplusplus
extern "C"
{
#endif


// ----------------------------------------------------------------------------
// Token Patterns


typedef struct tPattern_s tPattern;
typedef struct tpStep_s tpStep;

typedef struct
{
   tPattern   *pattern;   
   gToken     *t;
   gList      *inlist;
   tpStep     *step;
} tpHookParms;


// Op pointers
// This is used to filter tokens. This can be done either based on type.
typedef bool (*tpOpFunc)(tpHookParms *parms, int steptype);


// Standard ops:
// opNoComp
// Does not check anything returns true
bool opNoComp(tpHookParms *parms, int steptype);

// opCompType
// Compares the current token to the given type taken from the current step.
bool opCompType(tpHookParms *parms, int steptype);





// These codes are assigned to onTrue and onFalse and they define the actions
// the pattern takes depending on the return value of the stepOp. These make up
// the code half of the full action. See tpStepFlag for the flags half.
typedef enum
{
   // Continue to the next step
   scNext = 0x0,

   // Call current error handling hook
   scThrow,

   // Push the sub pattern on the operating stack and call it. When the sub
   // finishes, the execution will be continued from the next step.
   scPush,

   // Don't push the stack, just goto the label
   scGoto,

   // Pop the stack. Either continue execution from the previous stack entry
   // or end the pattern.
   scPop,

   // End execution. Completely stop it.
   scEnd,

   // There are six different codes which means the mask for codes needs to be
   // at least 0x7
   scCodeMask = 0x7
} tpStepCode;


// tpStepFlags are extra actions to be performed. These can be ORed together with
// a step code to form the full action to be performed. The codes start from bit
// 32 and work their way back down.
typedef enum
{
   // This flag signifies that the current token should be stored in the output
   // list which is passed to step hooks.
   sfStore       = 0x8000000,

   // This flag is only considered by scPush, and if set will reset the 
   // current stack pointer to it's entry point (or the stack entry fallback point)
   // beforing pushing the new stack entry
   sfBack     = 0x4000000,

   // This flag causes the step NOT to increment the token pointer within
   // the stream.
   sfStay   = 0x1000000,

   // Sets the current step as both the back point the fallback point for the current
   // stack entry.
   sfSetFB = 0x800000,

} tpStepFlags;


// The OR'd values of sfStay and sfSetFB
#define sfFBStay      sfSetFB|sfStay
// The ORed values of scPush and sfRestart
#define scPushBack    scPush|sfBack
// The ORed values of scNext and sfStay
#define scSkip        scNext|sfStay
// The ORed values of scGoto and sfStay
#define scGotoStay    scGoto|sfStay
// The ORed values of scPush and sfStay
#define scPushStay    scPush|sfStay


typedef enum 
{
   tpNoError,
   tpWarning,
   tpError,
   tpFatal,
} tpResultCode;



// These codes are returned by the error handling hooks and tell the pattern
// code what to do next.
typedef enum
{
   // Continue on with the current pattern.
   tpContinue,

   // Return execution to the nearest fallback point.
   tpFallback,

   // Pop the stack and return execution to the nearest fallback point.
   tpPopFallback,

   // Completely stop execution of the pattern.
   tpAbort
} tpHookCode;



typedef int (*tpStepHook)(tpHookParms *);


typedef tpHookCode (*tpErrHook)(tpHookParms *);



// A token pattern is basically a table of instructional "steps" which traverse
// the token stream and perform various actions based on the token types 
// encountered (although custom op functions can be created to proceed based on
// token content as well). 
typedef struct tpStep_s
{
   // Used for goto/calls
   char *label;

   // Action pointer 
   // This points to a function which checks the current type.
   tpOpFunc stepOp;

   // This is the type parameter supplied to stepOp
   int tokenType;

   // Actions to take based on the return value of the op function
   int  onTrue, onFalse;

   // Name of the label of the code refered to by tpStepCodes
   char *subLabel;

   // Hook function to call if stepOp returns true (ignored if NULL)
   tpStepHook hook;

   // Sets the current stack entry's error handling function (ignored if NULL)
   tpErrHook  errHook;

   // If executed action of the step is scThrow, this is the error level
   int elevel;

   // If executed action of the step is scThrow, this is the error message
   // It is a printf format string and will be called with the following input
   // - A string (with the stream name)
   // - An integer (line number)
   // - A second integer (column number)
   char *emsg;
};




typedef struct
{
   // Index of the current step
   int  stepIndex;

   // Index of the first step this stack entry called, or the most recent fallback
   // point set in the current stack entry.
   int  backIndex;

   // Index of the last step this stack entry called with the sfSetFallback flag
   // This is -1 if there have been none.
   int  fallbackIndex;

   // Current error handling function for this stack entry
   tpErrHook  errHook;
} tpStackEntry;


typedef struct tPattern_s
{
   // This is a pointer to the entire instruction set for the token pattern.
   tpStep *stepList;

   // This is the hash table that stores indecies for all the labels
   gHashTable *labelTable;

   // This is the token stream the tokens are being read from
   gTokenStream *tstream;

   // The index within the token stream the pattern is currently at
   int   i;

   // The stack for the pattern.
   gStack *stack;

   // Counts for errors and warnings
   int   ecount, wcount;
};



// tpNewPattern
// Creates a new tPattern object for use with the parsing functions.
tPattern *tpNewPattern(tpStep *stepList, gTokenStream *tstream);


// tpResetPattern
// Clears all internal data to it's initial state. Allows the same pattern to
// be run multiple times
//void tpResetPattern(tPattern *p);


// tpFreePattern
// Frees the given tPattern. This function will not attempt to free
// stepList but it will close the token stream.
void tpFreePattern(tPattern *p);


// tpClearInlist
// Clears out the given inlist, and clears the cache in the given token stream.
void tpClearInlist(tpHookParms *p);


// tpIncrementCount
// Increments either the warning count or the error count in the given pattern
// depending on the code supplied. Returns code for chaining.
int tpIncrementCount(tPattern *p, int code);


// tpExecutePattern
// Executes the given pattern object and returns the highest error level
// encountered during execution.
int tpExecutePattern(tPattern *p);


#ifdef __cplusplus
}
#endif


#endif