#!/home/ben/software/install/bin/perl

# Turn Markus Kuhn's "UTF-8 decoder capability and stress test" into a
# series of Perl tests.
# URL: L<http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt>

use warnings;
use strict;
use FindBin;
my $file = "$FindBin::Bin/UTF-8-test.txt";

# The file contains deliberate mistakes in UTF-8. If we open it using
# ":encoding(utf8)", Perl should choke on some bytes.

eval {
    use warnings FATAL => qw( all );
    open my $in, "<:encoding(utf8)", $file or die $!;
    while (<$in>) {
	;
    }
    close $in or die $!;
};

if (! $@) {
    die "Did not choke reading $file";
}

# We open it using ":raw".

my $testnumber;
my $subsection;
my $section;
my $start;
open my $in, "<:raw", $file or die $!;
while (<$in>) {
    if (/Here come the tests:/) {
	$start = 1;
    }
    # Skip non-test introductory material.
    if (! $start) {
	next;
    }
    s/\n$// or die "$file:$.: no linefeed";
#    chomp;

# The lines are 79 "characters" long where "characters" doesn't mean
# "bytes" but UTF-8 "characters", but then some of the UTF-8
# "characters" are faulty anyway, so what was the point of it?

#    if (length != 79) {
#	die "$file:$.: Bad length " . length;
#    }
    if (/^(\d+)\s+/) {
	$section = $1;
	print "Section $section\n";
    }
    if (/^(\d+\.\d+)\s+/) {
	$subsection = $1;
	if ($subsection !~ /\Q$section\E\.\d+/) {
	    die "$file:$.: $testnumber / $subsection mismatch";
	}
	print "Subsection $subsection\n";
    }
    if (/^(\d+\.\d+\.\d+)/) {
	$testnumber = $1;
	if ($testnumber !~ /\Q$subsection\E\.\d+/) {
	    die "$file:$.: $testnumber / $subsection mismatch";
	}
	print "Test number $testnumber\n";
    }
}
close $in or die $!;
