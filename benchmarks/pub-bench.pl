#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use FindBin '$Bin';
my @files = qw/short long words-array exp literals cpantesters/;

for my $file (@files) {
    my $bench = `$Bin/bench $Bin/$file.json`;
    $bench =~ s/^/    /gsm;
    print <<EOF;
=item $file.json

$bench

EOF
}


