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
my $status = system ("perl $Bin/test_json_parse.pl $Bin/n_structure_open_array_object.json");

# The parsing is meant to fail, so let's see what happens

cmp_ok ($status, '==', 256, "Got one status from exit of daxim.pl");
done_testing ();
