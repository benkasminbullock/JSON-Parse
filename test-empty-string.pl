#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib 'blib/lib';
use lib 'blib/arch';
use Json3 'parse_json';

my $json = parse_json ('{"buggles":"","bibbles":""}');
print $json->{buggles}, "\n";
print $json->{bibbles}, "\n";
$json->{buggles} .= "chuggles";
print $json->{bibbles}, "\n";
