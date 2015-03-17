# gParse #

Simple, general text parsing library. The project goal is to provide an easy to use API with a powerful set of tools for tokenizing, lexing, and parsing of information from text files. gParse can be used to parse anything from simple markup files to scripts. Although it is not a solution for all text parsing project, but it's designed to handle the most common ones.

Features include:
  * Completely C-based API. Anyone with a solid understanding of structs in C should be able to use gParse.
  * Does not rely on system specific code.
  * Released under the GNU LGPL license.

gParse is also part of the GUI library [Gooey a4](http://code.google.com/p/gooeya4/).

Currently the project only has windows a win32 implementation, we're currently seeking developers for other platforms (linux, ect) to help port the lib.

The current released version of the library is 0.1.1 (alpha)

**Notice** A major bug was discovered in 0.1.0 which caused token patterns to execute incorrectly. This has been fixed in 0.1.1.