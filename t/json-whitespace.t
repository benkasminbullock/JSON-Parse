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

BEGIN {
    eval {
		require JSON::Whitespace;
	} or do {
        plan skip_all => "I can't find JSON::Whitespace";
    };
}

my $json = '  { "key" : "value", "monkeys"  : 999 }  ';
my $out = JSON::Whitespace::json_no_space ($json);
is ($out, '{"key":"value","monkeys":999}', "Correctly stripped whitespace");
done_testing ();
