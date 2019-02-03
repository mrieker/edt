//+++2018-02-05
//    Copyright (C) 2001,2004,2009,2018 Mike Rieker, Beverly, MA USA
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
//---2018-02-05

/************************************************************************/
/*									*/
/*	set variable value						*/
/*									*/
/************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "edt.h"

extern char *strcasestr(char const *haystack, char const *needle);

char *(*xstrstr) () = strcasestr;
int  (*xstrncmp) () = strncasecmp;
int  eightbit = 0;
int  tabsize  = 8;
int  tabsoft  = 0;
int  linecrlf = 0;  // 0: neither; 1: reading; 2: writing; 3: read+write
int  linelf   = 3;  // 0: neither; 1: reading; 2: writing; 3: read+write

void cmd_set (char *cp)

{
  Buffer *buffer;
  char c, *p;
  int i, v;

  /* Set 8-bit char representation mode */

  if ((i = matchkeyword (cp, "8bit", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if ((i = matchkeyword (cp, "asis", 1)) > 0) {
      eightbit = 1;
    } else if ((i = matchkeyword (cp, "hex", 1)) > 0) {
      eightbit = 0;
    }
    if (!eoltest (cp + i)) goto usage;
    return;
  }

  /* Set autoshift count */

  if ((i = matchkeyword (cp, "autoshift", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    v = strtol (cp, &p, 10);
    if ((p == cp) || !eoltest (p)) goto usage;
    autoshift = v;
    return;
  }

  /* Set endings to crlf, lf, read, write, both */

  if ((i = matchkeyword (cp, "endings", 1)) > 0) {
    int *ptrset = NULL;
    int *ptrclr = NULL;
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if ((i = matchkeyword (cp, "crlf", 1)) > 0) {
      ptrset = &linecrlf;
      ptrclr = &linelf;
    }
    else if ((i = matchkeyword (cp, "lfonly", 1)) > 0) {
      ptrset = &linelf;
      ptrclr = &linecrlf;
    }
    else goto usage;
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if (*cp == 0) {
      *ptrset = 3;
      *ptrclr = 0;
      return;
    }
    if ((i = matchkeyword (cp, "both", 1)) > 0) {
      cp = skipspaces (cp + i);
      if (*cp != 0) goto usage;
      *ptrset = 3;
      *ptrclr = 0;
      return;
    }
    if ((i = matchkeyword (cp, "read", 1)) > 0) {
      cp = skipspaces (cp + i);
      if (*cp != 0) goto usage;
      *ptrset = 1;
      *ptrclr = 2;
      return;
    }
    if ((i = matchkeyword (cp, "write", 1)) > 0) {
      cp = skipspaces (cp + i);
      if (*cp != 0) goto usage;
      *ptrset = 2;
      *ptrclr = 1;
      return;
    }
    goto usage;
  }

  /* Set lfs (linefeeds) to hide or show */

  if ((i = matchkeyword (cp, "lfs", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if ((i = matchkeyword (cp, "hide", 1)) > 0) showlfs = 0;
    else if ((i = matchkeyword (cp, "show", 1)) > 0) showlfs = 1;
    else goto usage;
    if (!eoltest (cp + i)) goto usage;
    return;
  }

  /* Set numbers to hide or show */

  if ((i = matchkeyword (cp, "numbers", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if ((i = matchkeyword (cp, "auto", 1)) > 0) shownums = -1;
    else if ((i = matchkeyword (cp, "hide", 1)) > 0) shownums = 0;
    else if ((i = matchkeyword (cp, "show", 1)) > 0) shownums = 1;
    else goto usage;
    if (!eoltest (cp + i)) goto usage;
    return;
  }

  /* Set search to exact or generic */

  if ((i = matchkeyword (cp, "search", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp = skipspaces (cp + i);
    if ((i = matchkeyword (cp, "exact", 1)) > 0) {
      xstrstr  = strstr;
      xstrncmp = strncmp;
    } else if ((i = matchkeyword (cp, "generic", 1)) > 0) {
      xstrstr  = strcasestr;
      xstrncmp = strncasecmp;
    } else goto usage;
    if (!eoltest (cp + i)) goto usage;
    return;
  }

  /* Set tab stops */

  if ((i = matchkeyword (cp, "tab", 1)) > 0) {
    if (cp[i] > ' ') goto usage;
    cp += i;
    while (*(cp = skipspaces (cp)) != 0) {
      if ((i = matchkeyword (cp, "hard", 1)) > 0) {
        tabsoft = 0;
        cp += i;
        continue;
      }
      if ((i = matchkeyword (cp, "soft", 1)) > 0) {
        tabsoft = 1;
        cp += i;
        continue;
      }
      v = strtol (cp, &p, 10);
      if ((p == cp) || (*p > ' ')) goto usage;
      if ((v <= 0) || (v > MAXTABSIZE)) goto usage;
      tabsize = v;
      cp = p;
    }
    return;
  }

  /* Set buffer attributes (-output filename or -readonly) */

  if (*cp == '=') {
    for (p = ++ cp; (c = *cp) != 0; cp ++) if (strchr (bufnamechars, c) == NULL) break;
    if (cp == p) goto usage;
    if (*cp > ' ') goto usage;
    buffer = buffer_create (cp - p, p);
    cp = skipspaces (cp);
    if ((cp[7] <= ' ') && (strncasecmp (cp, "-output", 7) == 0)) {
      cp = skipspaces (cp + 7);
      p  = uptospace (cp);
      if (*p != 0) goto usage;
      buffer_setfile (buffer, cp);
      return;
    }
    if ((cp[9] <= ' ') && (strncasecmp (cp, "-readonly", 9) == 0)) {
      cp = skipspaces (cp + 9);
      if (*cp != 0) goto usage;
      buffer_setfile (buffer, NULL);
      return;
    }
    goto usage;
  }

usage:
  outerr (0, "set 8bit {asis | hex}\n\n");
  outerr (0, "set autoshift <count>\n\n");
  outerr (0, "set endings {crlf | lfonly} [both | read | write]\n\n");
  outerr (0, "set lfs {hide | show}\n\n");
  outerr (0, "set numbers {auto | hide | show}\n\n");
  outerr (0, "set search {exact | generic}\n\n");
  outerr (0, "set tab [<1..%d>] [{hard | soft}]\n\n", MAXTABSIZE);
  outerr (0, "set =<buffer> -output <filename>\n");
  outerr (0, "              -readonly\n\n");
}
