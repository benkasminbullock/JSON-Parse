#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use JSON::Create 'create_json';
my $undef;
my $hash = {'' => 10, '' => 99};
print create_json ($hash), "\n";
