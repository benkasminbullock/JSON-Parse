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
ok (-d "$Bin/../blib/lib", "Have built version to test against");
ok (-d "$Bin/../blib/arch", "Have built version to test against");
BEGIN: {
    use lib "$Bin/../blib/lib";
    use lib "$Bin/../blib/arch";
};
use JSON::Parse;
use JSON::Tokenize;
use JSON::Whitespace;
is ($JSON::Parse::VERSION, $JSON::Tokenize::VERSION, "Parse/Tokenize versions coincide");
is ($JSON::Parse::VERSION, $JSON::Whitespace::VERSION, "Parse/Whitespace versions coincide");
is ($JSON::Tokenize::VERSION, $JSON::Whitespace::VERSION, "Tokenize/Whitespace versions coincide");

done_testing ();
