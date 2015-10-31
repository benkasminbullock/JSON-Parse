#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use JSON::Tiny '0.54', qw(decode_json encode_json);
use JSON::Parse;
use JSON::Create;
sub l { print "\n@_:\n\n"; }
sub i { print "    @_\n"; }
my $cream = '{"clapton":true,"hendrix":false,"bruce":true,"fripp":false}';
my $jp = JSON::Parse->new ();
my $jc = JSON::Create->new ();
l "First do a round-trip of our modules";
i $jc->run ($jp->run ($cream));
l "Now do a round-trip of JSON::Tiny";
i encode_json (decode_json ($cream));
l "First, incompatible mode";
i 'tiny(parse):', encode_json ($jp->run ($cream));
i 'create(tiny):', $jc->run (decode_json ($cream));
l "Compatibility with JSON::Parse";
$jp->set_true (JSON::Tiny::true);
$jp->set_false (JSON::Tiny::false);
i 'tiny(parse):', encode_json ($jp->run ($cream));
l "Compatibility with JSON::Create";
$jc->bool ('JSON::Tiny::_Bool');
i 'create(tiny):', $jc->run (decode_json ($cream));
l "JSON::Parse and JSON::Create are still compatible too";
i $jc->run ($jp->run ($cream));
exit;
