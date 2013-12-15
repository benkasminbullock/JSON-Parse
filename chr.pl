#!/home/ben/software/install/bin/perl
use warnings;
use strict;

    no utf8;
    my $kani = '87f9';
    print utf8::is_utf8 ($kani), "\n";
    $kani = chr (hex ($kani));
    print utf8::is_utf8 ($kani), "\n";

