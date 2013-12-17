#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use JSON::Parse ':all';
no utf8;
# 蟹
my $kani = '["\u87f9"]';
my $p = parse_json ($kani);
print utf8::is_utf8 ($p->[0]), "\n";
# Prints 1, because it's upgraded regardless of the input string's
# flags.
