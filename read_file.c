//+++2009-12-04
//    Copyright (C) 2001,2009  Mike Rieker, Beverly, MA USA
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---2009-12-04

/************************************************************************/
/*									*/
/*  Read file into buffer						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edt.h"

void read_file (FILE *input_file, Buffer *buffer, Line *next_line)

{
  String *string;

  while ((string = readfileline (input_file)) != NULL) {
    line_insert (buffer, next_line, string);
  }
  fclose (input_file);
}

/************************************************************************/
/*									*/
/*  Read line from a file						*/
/*									*/
/*    Input:								*/
/*									*/
/*	input_file = file to read from					*/
/*									*/
/*    Output:								*/
/*									*/
/*	readfileline = NULL : end-of-file				*/
/*	               else : pointer to string				*/
/*									*/
/************************************************************************/

String *readfileline (FILE *input_file)

{
  int atbol, i, rc;
  String *string;

  static char *input_buff = NULL;
  static int   input_size = 0;

  if (input_buff == NULL) {			// set up a reasonable size to begin with
    input_size = 240;
    input_buff = malloc (input_size);
  }

  atbol = 1;					// we are at beg-of-line for tab detection
  i = 0;					// no characters written to input_buff yet
  while ((rc = fgetc (input_file)) >= 0) {	// read character from input file
    if ((rc == ' ') && tabsoft && atbol) {	// if spaces at beginning of line and we're doing soft tabs,
      if (atbol >= tabsize) {			//   if we've got a whole tab's worth of spaces,
        i -= tabsize - 1;			//     remove spaces from input buffer
        rc = '\t';				//     replace with a tab
        atbol = 0;				//     reset to count more spaces at beg-of-line
      }
      atbol ++;					//   one more space at beg-of-line seen
    } else {
      atbol = 0;				// not a space, not at beg-of-line any more
    }
    if (i >= input_size) {			// see if room for 'rc' char in input_buff
      input_size += 256;
      input_buff  = realloc (input_buff, input_size);
    }
    input_buff[i++] = rc;			// store 'rc' char on end of input_buff
    if (rc == '\n') break;			// stop if hit end-of-line character
  }
  string = NULL;
  if (i != 0) string = string_create (i, input_buff);
  return (string);
}
