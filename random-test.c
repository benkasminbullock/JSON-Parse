#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>

static void
croak (char * format, ...)
{
    exit (EXIT_FAILURE);
}

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

#include "Json3-common.c"
#include "Json3-perl.c"
#define NOPERL
#include "Json3-entry-points.c"
#include "Json3-random-test.c"


int main ()
{
    srandom (time (0));
    random_json ();
    return 0;
}
