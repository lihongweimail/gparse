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
//
// Test program for gParse

#include "gparse.h"
#include "math.h"


// Example 1:
// The calculator! 

// Two registers: main and memory
// Each command is on one line. The following commands are accepted:

// clear - Clears the main register
// add number - Adds the parameter to the main register
// sub number - Subtracts the parameter from the main register
// div number - Divides the number in the main register by the parameter
// mul number - Multiplies the number in the main register by the parameter
// root - Determines the sqare root of the main register
// square - Squares the number in the main register
// ma - Adds the current number in the main register to the memory register
// ms - Subtracts the number in the main register from the memory register
// mr - Sets the main register equal to the memory register
// mc - Clears the memory register
// exit - Quit the program

// This example only uses the tokenizer/lexer it does not use the token
// pattern feature.

// First, create a list of token types.
enum calcTypes
{
   // We don't want to start from zero because gTokenizer already defines
   // a whole set of basic token types, so we'll start from the marker at
   // the end of that list, which is tLastBaseToken.
   tClear = tLastBaseToken,
   tAdd,
   tSub,
   tDiv,
   tMul,
   tRoot,
   tSquare,
   tMA,
   tMS,
   tMR,
   tMC,
   tExit,
};


// Then, create a list of keywords for the tokenizer to find. The NULL at
// the end of each row simply tells the tokenizer not to replace the string
// with anything else. This could be used, for instance, to replace 'pi' with
// a tDecimal and a string of "3.14159265". This list must end with a NULL
// pointer.
gKeyword calcKeys[] = {
{"clear",  tClear, NULL},
{"add",    tAdd,   NULL},
{"sub",    tSub,   NULL},
{"div",    tDiv,   NULL},
{"mul",    tMul,   NULL},
{"root",   tRoot,  NULL},
{"square", tSquare,NULL},
{"ma",     tMA,    NULL},
{"ms",     tMS,    NULL},
{"mr",     tMR,    NULL},
{"mc",     tMC,    NULL},
{"exit",   tExit,  NULL},
{NULL}};


// This example won't require any symbol types or escape characters


// These will be the registers
double main_reg = 0.0, memory_reg = 0.0;
// Error count
int    ecount = 0;


// Prototype for the parsing function
int runCommand(gTokenStream *tokstrm);


int main (int argc, char **argv)
{
   // First, create the parameters object.
   gTokenParms  *parms = gNewParms();
   gTextStream  *txtstrm;
   gTokenStream *tokstrm;
   char         inbuffer[100];
   
   // Set some parameters:
   // Comments can be started with ' and continue to the end of the line...
   parms->comment1s = "'";
   parms->comment1e = "\n";
   // No other comment type is needed
   parms->comment2s = parms->comment2e = NULL;

   // Behavior flags. We want to ignore case, and mark newlines as tokens
   parms->flags = gIgnoreCase|gNewlineTokens;

   // The keyword list we made.
   parms->keywlist = calcKeys;
   
   printf("Welcome to Calc, the gParse test program!\n");

   txtstrm = gStreamFromMemory(inbuffer, 100, false);

   // Create the token stream.
   tokstrm = gCreateTokenStream(parms, txtstrm, "User input");

   while(1)
   {
      // Print out state
      printf("Main: %.2f, Memory: %.2f\nPlease enter command:", main_reg, memory_reg);

      // Get a command.
      fgets(inbuffer, 99, stdin);

      if(!runCommand(tokstrm))
         break;

      // Reset the token stream
      gResetTokenStream(tokstrm);

      printf("\n");
   }

   return 0;
}




int runCommand(gTokenStream *tokstrm)
{
   double parameter;
   gToken *tok;

   // getNextToken will always return a token, at the end of the file it
   // will just return an EOF marker every time it's called.
   tok = gGetNextToken(tokstrm);

   if(tok->type == tEOF || tok->type == tLineBreak)
      return 0;

   // Run the instructions!
   switch(tok->type)
   {
      // First the instructions that don't require a parameter
      case tExit:
         return 0;
      case tClear:
         printf("Main register cleared\n");
         main_reg = 0.0;
         break;
      case tRoot:
         printf("Square root of main register\n");
         main_reg = sqrt(main_reg);
         break;
      case tSquare:
         printf("Squaring main register\n");
         main_reg *= main_reg;
         break;
      case tMA:
         printf("Adding main to memory\n");
         memory_reg += main_reg;
         break;
      case tMS:
         printf("Subtracting main from memory\n");
         memory_reg -= main_reg;
         break;
      case tMR:
         printf("Recalling memory to main\n");
         main_reg = memory_reg;
         break;
      case tMC:
         printf("Memory register cleared\n");
         memory_reg = 0.0;
         break;
      case tAdd:
         // Get the next token! Should be our parameter!
         tok = gGetNextToken(tokstrm);

         // gToken is a struct. The actual string is in tok->token.
         if(tok->type == tInteger || tok->type == tDecimal)
         {
            parameter = atof(tok->token);
            printf("Adding %.2f to main register\n", parameter);
            main_reg += parameter;
         }
         else
         {
            printf("parameter was not a number\n");
            ecount++;
         }
         break;
      case tSub:
         // Pretty much same as add
         tok = gGetNextToken(tokstrm);
         if(tok->type == tInteger || tok->type == tDecimal)
         {
            parameter = atof(tok->token);
            printf("Subtracting %.2f from main register\n", parameter);
            main_reg -= parameter;
         }
         else
         {
            printf("parameter was not a number\n");
            ecount++;
         }
         break;
      case tDiv:
         tok = gGetNextToken(tokstrm);
         if(tok->type == tInteger || tok->type == tDecimal)
         {
            parameter = atof(tok->token);
            printf("Dividing main register by %.2f\n", parameter);
            main_reg /= parameter;
         }
         else
         {
            printf("parameter was not a number\n");
            ecount++;
         }
         break;
      case tMul:
         tok = gGetNextToken(tokstrm);
         if(tok->type == tInteger || tok->type == tDecimal)
         {
            parameter = atof(tok->token);
            printf("Multiplying main register by %.2f\n", parameter);
            main_reg *= parameter;
         }
         else
         {
            printf("parameter was not a number\n");
            ecount++;
         }
         break;
      default:
         printf("Error: Invalid command '%s'\n", tok->token);
         return 1;
   }

   // If there was a missing parameter, we might already be at a linebreak..
   if(tok->type != tLineBreak)
   {
      // The next token should be the linebreak...
      tok = gGetNextToken(tokstrm);

      if(tok->type != tLineBreak && tok->type != tEOF)
      {
         printf("\nLinebreak could not be found!\n");
         ecount++;
      }
   }

   return 1;
}