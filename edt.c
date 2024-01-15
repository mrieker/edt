//+++2024-01-15
//    Copyright (C) 2004,2005,2006,2008,2009,2011,2016,2018,2019,2021,2024  Mike Rieker, Beverly, MA USA
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
//---2024-01-15

/************************************************************************/
/*                                                                      */
/*  Editor main program                                                 */
/*                                                                      */
/************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "edt.h"

void cmd_exit (char *cmdpnt);
void cmd_open (char *cmdpnt);
void cmd_type (char *cmdpnt);
void cmd_write (char *cmdpnt);

typedef struct AddlOpen AddlOpen;
struct AddlOpen {
  AddlOpen *next;
  char *name;
};

Buffer *main_buffer;
char *input_name, *recover_name;
char *journal_name;                 /* journal filename */
const char *pn = "edt";             /* the program name (for error messages) */
FILE *journal_file, *recover_file;  /* journal file and recovery file */
Position cur_position;              /* current position */
int md5sum = 0;                     /* print md5sum when writing a file */
int showlfs = 0;                    /* 0: LF's are silent; 1: show LF's as <LF> */
int shownums = -1;                  /* -1: on for linemode, off for changemode; 0: always off; 1: always on */

static void process_initfile (char *init_name, char *default_init_name);

int main (int argc, char *argv[])

{
  AddlOpen *addlopen, *addlopens;
  char *init_name, *output_name, *p, *preinit_name;
  const char *cmdpnt;
  int i, rdonly, recover;
  String *cmdstr;

  if (argc > 0) pn = argv[0];

  fprintf (stderr, 
   "Copyright (C) Mike Rieker, Beverly, MA USA\n"
   "Version 2024-01-15, EDT comes with ABSOLUTELY NO WARRANTY\n"
   "EXPECT it to FAIL when someone's HeALTh or PROpeRTy is at RISk\n\n");
  fflush (stderr);

  /* Parse command line */

  addlopens    = NULL;
  init_name    = NULL;
  input_name   = NULL;
  journal_file = NULL;
  journal_name = NULL;
  output_name  = NULL;
  preinit_name = NULL;
  rdonly       = 0;
  recover      = 0;
  recover_file = NULL;
  recover_name = NULL;

  for (i = 1; i < argc; i ++) {

    /* -crlf : set line ending to cr/lf */

    if (strcasecmp (argv[i], "-crlf") == 0) {
      linecrlf = 3;
      linelf   = 0;
      continue;
    }

    /* -init <file> : process the given initialization file AFTER opening input file */

    if (strcasecmp (argv[i], "-init") == 0) {
      if (++ i >= argc) goto usage;
      if (argv[i][0] == '-') goto usage;
      if (init_name != NULL) goto usage;
      init_name = argv[i];
      continue;
    }

    /* -journal <file> : write journal to specified file */
    /* by default, it gets written to <output_file>.edtj */

    if (strcasecmp (argv[i], "-journal") == 0) {
      if (++ i >= argc) goto usage;
      if (argv[i][0] == '-') goto usage;
      journal_name = argv[i];
      continue;
    }

    /* -lfonly : set line ending to lfonly */

    if (strcasecmp (argv[i], "-lfonly") == 0) {
      linecrlf = 0;
      linelf   = 3;
      continue;
    }

    /* -md5sum : print md5sum when writing files */

    if (strcasecmp (argv[i], "-md5sum") == 0) {
      md5sum = 1;
      continue;
    }

    /* -noinit : don't process default init file */

    if (strcasecmp (argv[i], "-noinit") == 0) {
      if (init_name != NULL) goto usage;
      init_name = "";
      continue;
    }

    /* -nopreinit : don't process default init file */

    if (strcasecmp (argv[i], "-nopreinit") == 0) {
      if (preinit_name != NULL) goto usage;
      preinit_name = "";
      continue;
    }

    if (strcasecmp (argv[i], "-open") == 0) {
      if (++ i >= argc) goto usage;
      if (argv[i][0] == '-') goto usage;
      addlopen = malloc (sizeof *addlopen);
      addlopen->next = addlopens;
      addlopen->name = argv[i];
      addlopens = addlopen;
      continue;
    }

    /* -output <file> : write output to specified file */
    /* by default, it gets written to <input_file>     */

    if (strcasecmp (argv[i], "-output") == 0) {
      if (++ i >= argc) goto usage;
      if (argv[i][0] == '-') goto usage;
      output_name = argv[i];
      continue;
    }

    /* -preinit <file> : process the given initialization file BEFORE opening input file */

    if (strcasecmp (argv[i], "-preinit") == 0) {
      if (++ i >= argc) goto usage;
      if (argv[i][0] == '-') goto usage;
      if (preinit_name != NULL) goto usage;
      preinit_name = argv[i];
      continue;
    }

    /* -readonly : don't write an output file */

    if (strcasecmp (argv[i], "-readonly") == 0) {
      if (output_name == input_name) output_name = NULL;
      rdonly = 1;
      continue;
    }

    /* -recover [<file>] : process recovery from file           */
    /* by default, recovery is processed from <input_file>.edtj */

    if (strcasecmp (argv[i], "-recover") == 0) {
      recover = 1;
      if (i + 1 == argc) continue;
      if (argv[i+1][0] == '-') continue;
      if ((input_name != NULL) || (i + 2 <= argc)) recover_name = argv[++i];
      continue;
    }

    /* No more options */

    if (argv[i][0] == '-') goto usage;

    /* first and only parameter <file> : input filename */

    if (input_name == NULL) {
      input_name = argv[i];
      if (!rdonly) output_name = input_name;
      continue;
    }
    goto usage;
  }

  /* Open recovery file */

  if (recover && (input_name == NULL)) {
    fprintf (stderr, "no input file specified to recover\n");
    return (-1);
  }
  if (recover && (recover_name == NULL)) recover_name = os_makejnlname (input_name);
  if (recover_name != NULL) {
    recover_file = fopen (recover_name, "r");
    if (recover_file == NULL) {
      fprintf (stderr, "error opening recovery file %s: %s\n", recover_name, strerror (errno));
      return (-1);
    }
  }

  /* Create journal file */

  if ((output_name != NULL) && (journal_name == NULL)) journal_name = os_makejnlname (output_name);
  if (journal_name != NULL) {
    journal_file = os_crenewfile (journal_name);
    if (journal_file == NULL) {
      fprintf (stderr, "error creating journal file %s: %s\n", journal_name, strerror (errno));
      return (-1);
    }
  }

  /* Initialize os dependent routines.  No using stdin/stdout/stderr from now on. */

  os_initialization ();

  /* Process pre-initialization file */

  process_initfile (preinit_name, os_defaultpreinitname ());

  /* Open any additional files */

  while ((addlopen = addlopens) != NULL) {
    addlopens = addlopen->next;
    cur_position.buffer = NULL;
    cur_position.line   = NULL;
    cur_position.offset = 0;
    cmd_open (addlopen->name);
    free (addlopen);
    if (cur_position.line == NULL) return (-1);
  }

  /* If input file was given, read it into a buffer and mark that buffer for writing on exit */

  if (input_name != NULL) {
    p = input_name;
    if (rdonly) {
      p = malloc (strlen (input_name) + 12);
      strcpy (p, "-readonly ");
      strcat (p, input_name);
    }
    cur_position.buffer = NULL;
    cur_position.line   = NULL;
    cur_position.offset = 0;
    cmd_open (p);
    if (p != input_name) free (p);
    if (cur_position.buffer == NULL) return (-1);
    cur_position.line   = buffer_first_line (cur_position.buffer);
    cur_position.offset = 0;
  }

  /* Otherwise, allocate initial 'MAIN' buffer with no lines in it */

  else {
    cur_position.buffer = buffer_create (4, "MAIN");
    cur_position.line   = NULL;
    cur_position.offset = 0;
  }

  /* Either way, that is the one used by the EXIT command */

  main_buffer = cur_position.buffer;
  if (output_name != NULL) buffer_setfile (main_buffer, output_name);

  /* Process initialization file */

  process_initfile (init_name, os_defaultinitname ());

  /* Type out line at current position to begin with */

  cmd_type (".");

  /* Read and process commands until eof */

  i = 0;
  while (1) {
    cmdstr = jnl_readprompt ("\r\n*");
    if (cmdstr == NULL) {                 /* prompt then read command line */
      if (++ i < 3) outerr (0, "use either EXIT or QUIT to terminate\n");
      else {
        outerr (12, "%d EOF's in a row or fatal terminal error, exiting ...\n", i);
        cmd_exit ("");
        exit (-1);
      }
      continue;
    }
    i = 0;
    for (cmdpnt = string_getval (cmdstr); *cmdpnt != 0; cmdpnt ++) if (*cmdpnt > ' ') break;
    if (*cmdpnt == 0) cmd_type (".+1");               /* blank line means 'type .+1' */
    else ln_command (cmdpnt);                /* process command */
    string_delete (cmdstr);                  /* free the command string off */
  }

  /* Bad command line parameter */

usage:
  fprintf (stderr, "usage: %s [-crlf] [-init <init_file>] [-journal <journal_output>] [-lfonly] [-noinit] [-nopreinit] [-open '<addl_file> [<open_options>]'] [-output <output_file>] [-preinit <preinit_file>] [-readonly] [-recover [<journal_input>]] [<input_file>]\n", pn);
  return (-1);
}

static void process_initfile (char *init_name, char *default_init_name)
{
  char *buf;
  const char *cmdpnt;
  FILE *init_file;
  int len, siz;

  if (init_name == NULL) init_name = default_init_name;     /* if no -noinit or -init, get default name */
  if ((init_name != NULL) && (init_name[0] != 0)) {
    init_file = fopen (init_name, "r");            /* try to open init file */
    if (init_file != NULL) {
      siz = 256;                 /* start with 256 byte buffer */
      buf = malloc (siz);
      len = 0;                   /* don't have anything in it */
      while (fgets (buf + len, siz - len, init_file) != NULL) {   /* read onto end of what's there */
        len += strlen (buf + len);           /* get total length */
        if (len == 0) break;              /* stop if nothing there (eof?) */
        if (buf[len-1] != '\n') {            /* check for line terminator */
          siz += 256;                  /* didn't get the whole thing, increase buffer */
          buf  = realloc (buf, siz);            /* ... then loop to read more of the line */
        } else {
          buf[--len] = 0;              /* got terminator, remove it from buffer */
          cmdpnt = skipspaces (buf);            /* skip leading spaces */
          if (*cmdpnt != 0) ln_command (cmdpnt);      /* process command line */
          len = 0;                  /* empty line buffer for next read */
        }
      }
      fclose (init_file);
      free (buf);
    } else if (errno != ENOENT) {
      outerr (strlen (init_name) + strlen (strerror (errno)), "error opening init file %s: %s\n", init_name, strerror (errno));
      exit (-1);
    }
  }
}
