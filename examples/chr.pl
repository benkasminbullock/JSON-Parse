#!/home/ben/software/install/bin/perl
use warnings;
use strict;
no utf8;
my $kani = '87f9';
print "1: is utf8\n" if utf8::is_utf8 ($kani);
# prints nothing
$kani = chr (hex ($kani));
print "2: is utf8\n" if utf8::is_utf8 ($kani);
# prints "2: is utf8"
