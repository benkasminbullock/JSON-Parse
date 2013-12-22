#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use JSON::Parse ':all';

my $o = parse_json ('[true]');

if (\$o->[0] eq \$JSON::Parse::true) {
    print "True\n";
}
print \$o->[0], " ", \$JSON::Parse::true, "\n";
#print ref $t, "\n";
