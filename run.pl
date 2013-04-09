#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use File::Slurp;
use lib 'blib/lib';
use lib 'blib/arch';
use Json3 'parse_json';
my $file = $ARGV[0];
my $json = read_file ($file);
my $stuff = parse_json ($json);
