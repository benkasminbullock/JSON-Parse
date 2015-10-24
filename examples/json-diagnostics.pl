#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use JSON::Parse ':all';
$JSON::Parse::json_diagnostics = 1;
assert_valid_json ("{'not':'valid'}");

