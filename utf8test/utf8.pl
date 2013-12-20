#!/home/ben/software/install/bin/perl
use warnings;
use strict;

# http://cpansearch.perl.org/src/MARKF/Test-utf8-1.00/lib/Test/utf8.pm

# https://rt.cpan.org/Ticket/Display.html?id=91537

our $utf8_re = <<'REGEX' ;
        [\x{00}-\x{7f}]
      | [\x{c2}-\x{df}][\x{80}-\x{bf}]
      |         \x{e0} [\x{a0}-\x{bf}][\x{80}-\x{bf}]
      | [\x{e1}-\x{ec}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{ed} [\x{80}-\x{9f}][\x{80}-\x{bf}]
      | [\x{ee}-\x{ef}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{f0} [\x{90}-\x{bf}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      | [\x{f1}-\x{f3}][\x{80}-\x{bf}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{f4} [\x{80}-\x{8f}][\x{80}-\x{bf}][\x{80}-\x{bf}]
REGEX

my %ranges;

my $h = qr/([0-9a-f]{2})/;
my $hex = qr/\\x\{$h\}/i;

while ($utf8_re =~ /\[$hex-$hex\]/g) {
    $ranges{"$1$2"} = 1;
}
for my $range (sort {hex $a <=> hex $b} keys %ranges) {
    my ($lower, $upper) = ($range =~ /$h/g);
    my $macro = "BYTE_" . uc ($lower) . "_" . uc ($upper);
    print "#define $macro \\\n";
    my $l = hex $lower;
    my $u = hex $upper;
    printf "      0x%02X:", $l;
    for ($l + 1..$u - 1) {
	printf " case 0x%02X:", $_;
	if ((($_ - $l) % 7) == 6) {
	    print "\\\n";
	}
    }
    printf " case 0x%02X\n", $u;
}

my $fl = (__LINE__+3) .'"' . __FILE__ .'"';
my $switch = <<EOF;
# line $fl
switch (NEXTBYTE) {
case BYTE_00_7F:
    goto string_start;

case BYTE_C2_DF:
    goto byte_last_80_bf;

case 0xE0:
    goto byte23_a0_bf;

case BYTE_E1_EC:
    goto byte_penultimate_80_bf;

case 0xED:
    goto byte23_80_9f;

case BYTE_EE_EF:
    goto byte_penultimate_80_bf;

case 0xF0:
    goto byte24_90_bf;

case BYTE_F1_F3:
    goto byte24_80_bf;

case 0xF4:
    goto byte24_80_8f;

default:
    FAILUTF8;
}

byte_last_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto string_start;
default:
    FAILUTF8;
}

byte_penultimate_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte24_90_bf:

switch (NEXTBYTE) {

case BYTE_90_BF:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}

byte23_80_9f:

switch (NEXTBYTE) {

case BYTE_80_9F:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte23_a0_bf:

switch (NEXTBYTE) {

case BYTE_A0_BF:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte24_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}

byte24_80_8f:

switch (NEXTBYTE) {

case BYTE_80_8F:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}
EOF
$switch =~ s/(^|\n)(.*)/$1    $2/g;
my $stuff =<<'EOF';
#include "stdio.h"
#define NEXTBYTE (c=*utf8++)
#define FAILUTF8 printf ("Failed with %c '%02X'.\n", c, c); return -1

int parse_utf8 (unsigned char * utf8, int length)
{
    unsigned char c;
    unsigned char * p;
    p = utf8;
string_start:
    if (utf8 - p >= length) {
        printf ("finished.\n");
        return 0;
    }
    printf ("%p %p %X %d\n", p, utf8, utf8 - p, utf8 - p);
EOF
$stuff .= $switch;
$stuff .= <<'EOF';
#line XXX "utf8.c"
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

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
read_whole_file (const char * file_name)
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
    return contents;
}

int main ()
{
    unsigned char * file_contents;
    file_contents = read_whole_file ("test.txt");
    parse_utf8 (file_contents, 44485);
    return 0;
}

EOF

my @lines = split /\n/, $stuff;
for my $i (0..$#lines) {
    $lines[$i] =~ s/XXX/$i + 1/e;
}
$stuff = join "\n", @lines;
print "$stuff\n";
exit;

