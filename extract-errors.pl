#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use C::Utility qw/remove_quotes/;
use C::Tokenize qw/$string_re/;
my @files = <Json3-*.c>;
for my $file (@files) {
    local $/ = ';';
    open my $in, "<", $file or die $!;
    while (<$in>) {
	if (/fail(badinput|bug|resources)\s*\(.*?parser,\s*($string_re)/ms) {
	    my $type = $1;
	    my $error = remove_quotes ($2);
	    print "error: $error\ntype: $type\n";
	}
    }
    close $in or die $!;
}
