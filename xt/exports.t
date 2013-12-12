# This tests the EXPORTS section of the pod documentation lists all
# the exported functions, and that all the listed functions are
# exported.

use warnings;
use strict;
use JSON::Parse;
use Pod::Select;
use FindBin;
use Test::More;

# Find out what is really exported from the module by poking our nose
# in where it's not supposed to go.

my @exports = @JSON::Parse::EXPORT_OK;
my %reallyexports;
@reallyexports{@exports} = @exports;

# The Master Pod File:

my $pod = "$FindBin::Bin/../lib/JSON/Parse.pod";
die "no $pod found" if ! -f $pod;

# Pod text goes in here.

my $exports_pod;

# Deal with stupid pod select interface by opening string as file handle.

open my $out, ">", \$exports_pod or die $!;

# Stupid pod select interface follows. Remember kids: never use
# something simple like a hash/key argument list when you can use
# something complicated like a hash reference with minus signs and
# array references, blah, blah, followed by the input file
# itself. Yikes!

podselect ({
    -output => $out,
    -sections => ['EXPORTS'],
}, $pod);

# Check we have listed everything in the pod.

my %exports;
while ($exports_pod =~ m!L</(\w*)>!g) {
    $exports{$1} = 1;
}
for my $function (@exports) {
    ok ($exports{$function}, "listed $function");
}

# Check everything listed in the pod really is exported.

for my $function (keys %exports) {
    ok ($reallyexports{$function}, "really does export $function");
}

# Everything is dandy.

done_testing ();
exit;
