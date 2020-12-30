#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use File::Slurper qw!read_text write_text!;

my $version = '0.57_01';
my $newversion = '0.57_02';

my @files = qw!
Changes
lib/JSON/Parse.pm
lib/JSON/Tokenize.pm
lib/JSON/Whitespace.pm
!;

for my $file (@files) {
    my $bfile = "$Bin/$file";
    my $text = read_text ($bfile);
    if ($text =~ s/\Q$version\E/$newversion/g) {
	print "$file OK\n";
	# write_text ($bfile, $text);
    }
    else {
	warn "$file failed";
    }
}
