# Check that the "SEE ALSO" part of the pod doesn't contain the same
# module twice and is sorted so that the modules mentioned are in
# case-insensitive alphabetical order.

use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use lib "$Bin/../build/";
use JPB;
use Test::More;
my $builder = Test::More->builder;
binmode $builder->output,         ":utf8";
binmode $builder->failure_output, ":utf8";
binmode $builder->todo_output,    ":utf8";
binmode STDOUT, ":encoding(utf8)";
binmode STDERR, ":encoding(utf8)";

# Check no duplicates

# Read the file in & extract the section

my $text = see_also ();

my %modules;
while ($text =~ /$mod_re/g) {
    ok (! $modules{$1}, "Entry for $1 is not a duplicate");
    $modules{$1} = 1;
}

# Check each subsection is in case-insensitive alphabetical order

while ($text =~ /=item\s+(.*?)=over(.*?)=back/gsm) {
    my $name = $1;
    my $section = $2;
    $name =~ s!\n.*$!!gs;
    if ($name =~ /RFC/) {
	next;
    }
    my @sm;
    while ($section =~ /$mod_re/g) {
	push @sm, $1;
    }
    my @sms = sort {uc $a cmp uc $b} @sm;
    is_deeply (\@sm, \@sms, "Modules in section $name are sorted");
}

done_testing ();
