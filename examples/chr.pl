#!/home/ben/software/install/bin/perl
use warnings;
use strict;

no utf8;
my $kani = '87f9';
print utf8::is_utf8 ($kani), "\n";
# prints a blank line
$kani = chr (hex ($kani));
print utf8::is_utf8 ($kani), "\n";
# prints 1

