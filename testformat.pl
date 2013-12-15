#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib 'blib/lib';
use lib 'blib/arch';

use JSON::Parse 'validate_json';
my $contains_null = "[\0\0\"pupparoon baba\"]";
eval {
    validate_json ($contains_null);
};
print "$@\n";
my $z = '  {"baba"';
eval {
    validate_json ($z);
};
print "$@\n";
eval {
    validate_json ('[1234');
};
print "$@\n";
