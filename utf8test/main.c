#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define NEXTBYTE (c=*utf8++)
#define FAILUTF8						\
    printf ("%d: Failed with %c '%02X'.\n", line, c, c);	\
    goto string_start

int parse_utf8 (unsigned char * utf8, int length)
{
    unsigned char c;
    unsigned char * p;
    unsigned char * q;
    int bytes;
    int line = 1;
    q = p = utf8;
string_start:
    if (utf8 - p >= length) {
        printf ("finished.\n");
        return 0;
    }
    bytes = utf8 - q;
    /*
    printf ("%p %p %X %d bytes: %d %.*s\n",
	    q, utf8, utf8 - p, utf8 - p, bytes, bytes, q);
    */
    q = utf8;
#include "utf8.c"
}

/* This routine returns the size of the file it is called with. */

static unsigned
get_file_size (const char * file_name)
{
    struct stat sb;
    if (stat (file_name, & sb) != 0) {
        fprintf (stderr, "'stat' failed for '%s': %s.\n",
                 file_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    return sb.st_size;
}

/* This routine reads the entire file into memory. */

static unsigned char *
read_whole_file (const char * file_name, unsigned * size)
{
    unsigned s;
    unsigned char * contents;
    FILE * f;
    size_t bytes_read;
    int status;

    s = get_file_size (file_name);
    contents = malloc (s + 1);
    if (! contents) {
        fprintf (stderr, "Not enough memory.\n");
        exit (EXIT_FAILURE);
    }

    f = fopen (file_name, "r");
    if (! f) {
        fprintf (stderr, "Could not open '%s': %s.\n", file_name,
                 strerror (errno));
        exit (EXIT_FAILURE);
    }
    bytes_read = fread (contents, sizeof (unsigned char), s, f);
    if (bytes_read != s) {
        fprintf (stderr, "Short read of '%s': expected %d bytes "
                 "but got %d: %s.\n", file_name, s, bytes_read,
                 strerror (errno));
        exit (EXIT_FAILURE);
    }
    status = fclose (f);
    if (status != 0) {
        fprintf (stderr, "Error closing '%s': %s.\n", file_name,
                 strerror (errno));
        exit (EXIT_FAILURE);
    }
    * size = s;
    return contents;
}

int main (int argc, char ** argv)
{
    unsigned char * file_contents;
    unsigned size;
    char * filename;
    if (argc > 1) {
	filename = argv[1];
    }
    file_contents = read_whole_file (filename, & size);
    parse_utf8 (file_contents, size);
    return 0;
}

