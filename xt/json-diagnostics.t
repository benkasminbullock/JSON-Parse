#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use JSON::Parse ':all';
use Test::More;
$JSON::Parse::json_diagnostics = 1;
eval {
    assert_valid_json ("baba");
};
my $error = $@;
note ($@);
$JSON::Parse::json_diagnostics = 0;
eval {
    assert_valid_json ($error);
};
ok (! $@);

note ($@);
done_testing ();
