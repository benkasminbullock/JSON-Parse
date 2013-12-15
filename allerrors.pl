#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use lib '/home/ben/projects/Json3/blib/lib';
use lib '/home/ben/projects/Json3/blib/arch';
use JSON::Parse 'assert_valid_json';
my @allerrors = (
{
    in => '   ',
},
{
    in => '{,"bad":"bad"}',
    bad_byte => 2,
},
{
    in => '[,"bad","bad"]',
    bad_byte => 2,
},
{
    in => '{"bad",:"bad"}',
    bad_byte => 7,
},
{
    in => '{"bad":,"bad"}',
    bad_byte => 8,
},
{
    in => '{"bad":"bad",}',
    bad_byte => 13,
},
{
    in => '["bad","bad",]',
    bad_byte => 13,
},
{
    in => '["bad" "bad"]',
    bad_byte => 8,
},
{
    in => '{"bad":"bad"}}',
    bad_byte => 14,
},
{
    in => '["bad","bad"]]',
    bad_byte => 14,
},
{
    in => '["' . "pupparoon\0\0 baba". '"]',
    bad_byte => 12,
},
{
    in => '["' . chr (07) . '"]',
    bad_byte => 3,
},
{
    in => '[{"\a":"baba"}]',
    bad_byte => 5,
},
{
    in => '[truk]',
    bad_byte => 5,
},
{
    in => '[--1]',
    bad_byte => 3,
},
{
    in => '[01]',
    bad_byte => 3,
},
{
    in => '[+1]',
    bad_byte => 2,
},
{
    in => '[0.1e++3]',
    bad_byte => 7,
},
{
    in => '[1.0e1.0]',
    bad_byte => 7,
},
{
    in => '[1234567',
},
{
    in => '["a":1]',
    bad_byte => 5,
},
{
    in => '{1,2,3}',
    bad_byte => 2,
},
{
    in => '[1,2,3}',
    bad_byte => 7,
},
{
    in => '{"go":{"buddy":{"go":{"buddy":',
},
{
    in => '{"gobuggs}',
},
{
    in => '["\uNOTHEX"]',
    bad_byte => 5,
},
{
    in => '["\uABC',
},
{
    in => '["\uD834monkey\uDD1E"]',
    bad_byte => 9,
},
);
for my $error (@allerrors) {
    my $in = $error->{in};
    eval {
	assert_valid_json ($in);
    };
    my $msg = $@;
    chomp ($msg);
    my $cleanin = $in;
    $cleanin =~ s/([\x00-\x1F])/sprintf ("\\x%02X", ord ($1))/ge;
#    if ($msg !~ /array|object/) {
    print "$cleanin: $msg\n";
    if ($msg =~ m!byte (\d+)/(\d+)!) {
	my $byte = $1;
	my $length = $2;
	if ($error->{bad_byte}) {
	    if ($error->{bad_byte} != $byte) {
		print "$error->{bad_byte} != $byte\n";
	    }
	}
	else {
	    print "No byte for error / $byte\n";
	}
	if (length ($in) != $length) {
	    print "" . (length $in) . " != $length\n";
	}
    }
#}
}
