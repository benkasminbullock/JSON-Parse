#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use Path::Tiny;
use JSON::Parse 'parse_json_safe';
my @j = <*.json>;
for my $j (@j) {
    print "$j\n";
    my $ji = path ($j)->slurp_utf8 ();
    parse_json_safe ($ji);
    exit;
}
