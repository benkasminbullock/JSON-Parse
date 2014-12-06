/* This is a C version of "random_json()". It can be compiled without
   reference to Perl.

   It prints one string of random JSON and exits. */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>

static void
croak (char * format, ...);

#include "unicode.h"
#include "unicode.c"
/* Get the "extra" stuff like "setjmp" by defining this. */
#define TESTRANDOM
/* These are Perl macros for Perl's own malloc/free/realloc. We are
   defining them here because this is a C program not linked to any
   Perl library. */
#define Newx(a,b,c) a = malloc (b*sizeof (c))
#define Renew(a,b,c) a = realloc (a, b*sizeof (c))
#define Safefree(a) free (a)

#define NOPERL
#include "json-common.c"
#include "json-perl.c"
#include "json-entry-points.c"
#include "json-random-test.c"

static void
croak (char * format, ...)
{
    va_list args;
    va_start (args, format);
    vfprintf (stderr, format, args);
    va_end (args);
    fprintf (stderr, "\n");
    exit (EXIT_FAILURE);
}


int main ()
{
    srandom (time (0));
    random_json ();
    return 0;
}
