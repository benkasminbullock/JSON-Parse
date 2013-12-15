#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib 'blib/lib';
use lib 'blib/arch';
use JSON::Parse 'assert_valid_json';
my $bad = '{"bad":"forgot the end quotes}';
assert_valid_json ($bad);
