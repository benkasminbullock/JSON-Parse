#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib '/home/ben/projects/Json3/blib/lib';
use lib '/home/ben/projects/Json3/blib/arch';
use JSON::Parse 'parse_json';

my $inputU = '{"x":1, "y":2, "z":"\u304f\u305d\n\u8aad\u307f\u305f\u3044\u304c\u8cb7\u3063\u305f\u3089\u8ca0\u3051\u306a\u6c17\u304c\u3059\u308b\n\u56f3\u66f8\u9928\u306b\u51fa\u306d\u30fc\u304b\u306a"}';
my $input = '{"x":1, "y":2, "z":"mmmooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"}';

foreach my $i(1..2) {
#foreach my $i(1..1E8) {
    my $perl = parse_json($input);
    undef $perl;
}
