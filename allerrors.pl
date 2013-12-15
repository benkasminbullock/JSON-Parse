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
    parsing => 'object',
},
{
    in => '[,"bad","bad"]',
    bad_byte => 2,
    parsing => 'array',
},
{
    in => '{"bad",:"bad"}',
    bad_byte => 7,
    parsing => 'object',
},
{
    in => '{"bad":,"bad"}',
    bad_byte => 8,
    parsing => 'object',
},
{
    in => '{"bad":"bad",}',
    bad_byte => 13,
    parsing => 'object',
},
{
    in => '["bad","bad",]',
    bad_byte => 13,
    parsing => 'array',
},
{
    in => '["bad" "bad"]',
    bad_byte => 8,
    parsing => 'array',
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
    parsing => 'string',
},
{
    in => '["' . chr (07) . '"]',
    bad_byte => 3,
    parsing => 'string',
},
{
    in => '[{"\a":"baba"}]',
    bad_byte => 5,
    parsing => 'string',
},
{
    in => '[truk]',
    bad_byte => 5,
    parsing => 'literal',
},
{
    in => '[--1]',
    bad_byte => 3,
    parsing => 'number',
},
{
    in => '[01]',
    bad_byte => 3,
    parsing => 'number',
},
{
    in => '[+1]',
    bad_byte => 2,
},
{
    in => '[0.1e++3]',
    bad_byte => 7,
    parsing => 'number',
},
{
    in => '[1.0e1.0]',
    bad_byte => 7,
    parsing => 'number',
},
{
    in => '[1234567',
    parsing => 'number',
},
{
    in => '["a":1]',
    bad_byte => 5,
    parsing => 'array',
},
{
    in => '{1,2,3}',
    bad_byte => 2,
    parsing => 'object',
},
{
    in => '[1,2,3}',
    bad_byte => 7,
    parsing => 'array',
},
{
    in => '{"go":{"buddy":{"go":{"buddy":',
    parsing => 'object',
},
{
    in => '{"gobuggs}',
    parsing => 'string',
},
{
    in => '["\uNOTHEX"]',
    bad_byte => 5,
    parsing => 'unicode escape',
},
{
    in => '["\uABC',
    parsing => 'unicode escape',
},
{
    in => '["\uD834monkey\uDD1E"]',
    bad_byte => 9,
    parsing => 'string',
},
{
    in => '[1.0e1+0]',
    bad_byte => 7,
    parsing => 'number',
},
);
for my $error (@allerrors) {
    my $in = $error->{in};
    eval {
	assert_valid_json ($in);
    };
    my $msg = $@;
    chomp ($msg);
    $msg =~ s/at allerrors\.pl.*$//;
    my $cleanin = $in;
    $cleanin =~ s/([\x00-\x1F])/sprintf ("\\x%02X", ord ($1))/ge;
#    if ($msg !~ /array|object/) {
    print "input: '$cleanin'\nresponse: $msg\n";
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
    my $parsing = $error->{parsing};
    if ($parsing) {
	if ($msg =~ /parsing\s+(unicode escape|\w+)/) {
	    if ($1 ne $parsing) {
		print "Got $1 expected $parsing\n";
	    }
	}
	else {
	    print "No parsing $parsing in message.\n";
	}
    }
    if ($msg =~ /unexpected character/i && $parsing ne 'literal') {
	if ($msg !~ /expecting/) {
	    die "No expecting.\n";
	}
    }
    print "\n";
#}
}
