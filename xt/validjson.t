use FindBin '$Bin';
use lib "$Bin";
use JPXT;

use Deploy 'do_system';
use IPC::Run3;

my $script = "$Bin/../script/validjson";
my $lib = "-I $Bin/../blib/lib -I $Bin/../blib/auto";
my @y = <$Bin/jpts/y_*>;
my @n = <$Bin/jpts/n_*>;

for my $y (@y) {
    if (daft_test ($y)) {
	next;
    }
    run3 ("perl $lib $script $y -v", undef, \my $out, \my $error);
    ok (! $error, "no errors");
    if ($error) {
	note "Got error '$error'";
    }
    ok ($out =~ m!\Q$y\E.*is valid JSON!, "$y is valid");
    unlike ($out, qr/error/i, "no error with $y");
}

for my $n (@n) {
    run3 ("perl $lib $script $n -v", undef, \my $out, \my $error);
    ok (! $error, "no errors");
    if ($error) {
	note "Got error '$error'";
    }
    like ($out, qr/error/i, "Got error with $n");
    unlike ($out, qr!\Q$n\E.*is valid JSON!, "$n is not valid");
}

done_testing ();
