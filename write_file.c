//+++2016-12-22
//    Copyright (C) 2001,2006,2009,2013,2016  Mike Rieker, Beverly, MA USA
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
//---2016-12-22

/************************************************************************/
/*									*/
/*  Write range of lines to file					*/
/*									*/
/*    Input:								*/
/*									*/
/*	out_name = name of file to write the lines to			*/
/*	beg_line = starting line to write (inclusive)			*/
/*	end_line = ending line to write (inclusive)			*/
/*									*/
/*    Output:								*/
/*									*/
/*	write_file = 0 : write error					*/
/*	             1 : successful					*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edt.h"
#include "md5.h"

static int write_line_md5sum (Line *line, FILE *file, struct md5_ctx *md5ctx);
static int fputs_md5sum (char const *s, FILE *file, struct md5_ctx *md5ctx);

int write_file (const char *out_name, Line *beg_line, Line *end_line)

{
  FILE *out_file;
  Line *line;
  struct md5_ctx md5ctx;
  uLong count;

  if (os_readonlyfile (out_name)) {
    outerr (strlen (out_name), "output file %s is readonly\n", out_name);
    return (0);
  }

  out_file = os_crenewfile (out_name);
  if (out_file == NULL) return (0);

  md5_init_ctx (&md5ctx);

  count = 0;
  for (line = beg_line; line != NULL; line = line_next (line)) {
    if (write_line_md5sum (line, out_file, &md5ctx) < 0) {
      outerr (strlen (out_name) + strlen (strerror (errno)), "error writing file %s: %s\n", out_name, strerror (errno));
      fclose (out_file);
      return (0);
    }
    count ++;
    if (line == end_line) break;
  }

  if (fclose (out_file) < 0) {
    outerr (strlen (out_name) + strlen (strerror (errno)), "error closing file %s: %s\n", out_name, strerror (errno));
    return (0);
  }

  outerr (strlen (out_name) + 12, "%s %u line%s\n", out_name, count, (count != 1) ? "s" : "");
  if (md5sum) {
    uByte md5res[16];
    md5_finish_ctx (&md5ctx, md5res);
    outerr (48, "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X  md5sum\n", 
      md5res[ 0], md5res[ 1], md5res[ 2], md5res[ 3], md5res[ 4], md5res[ 5], md5res[ 6], md5res[ 7], 
      md5res[ 8], md5res[ 9], md5res[10], md5res[11], md5res[12], md5res[13], md5res[14], md5res[15]);
  }
  return (1);
}

/* Write single line to output file */

int write_line (Line *line, FILE *file)
{
  return write_line_md5sum (line, file, NULL);
}

static int write_line_md5sum (Line *line, FILE *file, struct md5_ctx *md5ctx)
{
  char *b, c, *q;
  char const *p;
  char const *s;
  int ntabs;

  s = string_getval (line_string (line));
  if (!tabsoft) return (fputs_md5sum (s, file, md5ctx));

  ntabs = 0;
  for (p = s; (c = *p) != 0; p ++) {
    ntabs += (c == '\t');
  }
  if (ntabs == 0) return (fputs_md5sum (s, file, md5ctx));

  b = malloc (strlen (s) + (tabsize - 1) * ntabs + 1);
  q = b;
  for (p = s; (c = *p) != 0; p ++) {
    if (c == '\t') {
      do *(q ++) = ' ';
      while ((q - b) % tabsize != 0);
    } else {
      *(q ++) = c;
    }
  }
  *q = 0;
  ntabs = fputs_md5sum (b, file, md5ctx);
  free (b);
  return (ntabs);
}

static int fputs_md5sum (char const *s, FILE *file, struct md5_ctx *md5ctx)
{
  if (md5ctx != NULL) {
    md5_process_bytes (s, strlen (s), md5ctx);
  }
  return fputs (s, file);
}
