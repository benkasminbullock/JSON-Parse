use warnings;
use strict;
use Test::More;
use Deploy qw/file_slurp do_system/;
use FindBin;
my $efile = "$FindBin::Bin/errors";
my $ofile = "$FindBin::Bin/output";
my @examples = <$FindBin::Bin/../examples/*.pl>;
rmfiles ($efile, $ofile);
for my $example (@examples) {
    my $perl = file_slurp ($example);
    if ($perl =~ /#\s*prints\s*"([^"]+)"/i) {
	my $prints = $1;
	do_system ("perl -I $FindBin::Bin/../blib/lib -I $FindBin::Bin/../blib/arch $example > $ofile 2> $efile");
	ok (-f $ofile, "Made $ofile");
	ok (! -s $efile, "Didn't make $efile");
	if (-s $efile) {
	    print file_slurp ($efile);
	    exit;
	}
	my $output = file_slurp ($ofile);
	like ($output, qr/\Q$prints/, "Got $prints in output");
    }
    rmfiles ($ofile, $efile);
}
done_testing ();
exit;

sub rmfiles
{
    my (@files) = @_;
    for my $file (@files) {
	if (-f $file) {
	    unlink $file or die $!;
	}
    }
}
