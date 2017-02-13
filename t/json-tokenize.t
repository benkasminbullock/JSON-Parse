# Test for JSON::Tokenize

use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use Test::More;
my $builder = Test::More->builder;
binmode $builder->output,         ":utf8";
binmode $builder->failure_output, ":utf8";
binmode $builder->todo_output,    ":utf8";
binmode STDOUT, ":encoding(utf8)";
binmode STDERR, ":encoding(utf8)";
use JSON::Tokenize;
use JSON::Tokenize ':all';
my $input = '{"tuttie":["fruity", true, 100]}';
my $token = tokenize_json ($input);
is (tokenize_type ($token), 'object');
my $child = tokenize_child ($token);
is (tokenize_type ($child), "string");
my $next = tokenize_next ($child);
is (tokenize_type ($next), "colon");
done_testing ();
