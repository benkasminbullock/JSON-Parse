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
use File::Slurper 'read_text';
use Test::More;

my $pod = "$Bin/../lib/JSON/Parse.pod.tmpl";
my $text = read_text ($pod);
if (! ($text =~ s!^.*=head1 SEE ALSO(.*)=head1.*$!$1!gsm)) {
    die "Could not extract see also";
}
my %modules;
while ($text =~ /=item\s+L<(.*)>/g) {
    ok (! $modules{$1}, "Entry for $1 is not a duplicate");
    $modules{$1} = 1;
}
while ($text =~ /=item\s+(.*?)=over(.*?)=back/gsm) {
    my $name = $1;
    my $section = $2;
    $name =~ s!\n.*$!!gs;
    if ($name =~ /RFC/) {
	next;
    }
    my @sm;
    while ($section =~ /=item\s+L<(.*)>/g) {
	push @sm, $1;
    }
#    print "@sm\n\n";
    my @sms = sort {uc $a cmp uc $b} @sm;
    is_deeply (\@sm, \@sms, "Modules in section $name are sorted");
}
# Check no duplicates

done_testing ();
