use warnings;
use strict;
use Test::More;
use FindBin '$Bin';
use Perl::Build qw/get_version/;

# Check that the OPTIMIZE flag is not set in Makefile.PL. This causes
# errors on various other people's systems when compiling.

my $file = "$Bin/../Makefile.PL";
open my $in, "<", $file or die $!;
while (<$in>) {
    if (/-Wall/) {
	like ($_, qr/^\s*#/, "Commented out -Wall in Makefile.PL");
    }
}
close $in or die $!;

# Check that the examples have been included in the distribution (they
# were not included up to version 0.45 due to regex errors in
# MANIFEST.SKIP).

my $version = get_version (base => "$Bin/..");
my $distrofile = "$Bin/../JSON-Parse-$version.tar.gz";
if (! -f $distrofile) {
    die "No $distrofile";
}
my @tgz = `tar tfz $distrofile`;
my $has_collide_pl;
for (@tgz) {
    if (/collide\.pl/) {
	$has_collide_pl = 1;
    }
}
ok ($has_collide_pl, "collide.pl example is in tar file");


done_testing ();
