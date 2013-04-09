#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib 'blib/lib';
use lib 'blib/arch';
use Json3 'parse_json';
use Devel::Peek;
my $j = parse_json ('[false, false]');
Dump $j;
