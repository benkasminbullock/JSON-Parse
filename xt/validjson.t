use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use Test::More;
use Deploy 'do_system';
use IPC::Run3;
my $builder = Test::More->builder;
binmode $builder->output,         ":utf8";
binmode $builder->failure_output, ":utf8";
binmode $builder->todo_output,    ":utf8";
binmode STDOUT, ":encoding(utf8)";
binmode STDERR, ":encoding(utf8)";

my $script = "$Bin/../script/validjson";
my $lib = "-I $Bin/../blib/lib -I $Bin/../blib/auto";
my @y = <$Bin/../t/jpts/y_*>;
my @n = <$Bin/../t/jpts/n_*>;
for my $y (@y) {
    run3 ("perl $lib $script $y -v", undef, \my $out, \my $error);
    ok (! $error, "no errors");
    ok ($out =~ m!\Q$y\E.*is valid JSON!, "$y is valid");
    unlike ($out, qr/error/i, "no error with $y");
}
for my $n (@n) {
    run3 ("perl $lib $script $n -v", undef, \my $out, \my $error);
    ok (! $error, "no errors");
    like ($out, qr/error/i, "Got error with $n");
    unlike ($out, qr!\Q$n\E.*is valid JSON!, "$n is not valid");
}

done_testing ();
