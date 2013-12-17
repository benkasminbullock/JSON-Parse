#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Template;
use FindBin;

# Names of the input and output files containing the documentation.

my $pod = 'Parse.pod';
my $input = "$FindBin::Bin//lib/JSON/$pod.tmpl";
my $output = "$FindBin::Bin/lib/JSON/$pod";

# Template toolkit variable holder

my %vars;

my $tt = Template->new (
    ABSOLUTE => 1,
    INCLUDE_PATH => [
	$FindBin::Bin,
	'/home/ben/projects/Perl-Build/lib/Perl/Build/templates',
	"$FindBin::Bin/examples",
    ],
    ENCODING => 'UTF8',
    FILTERS => {
        xtidy => [
            \& xtidy,
            0,
        ],
    },
);

$tt->process ($input, \%vars, $output, {binmode => 'utf8'})
    or die '' . $tt->error ();

exit;

# This removes some obvious boilerplate from the examples, to shorten
# the documentation, and indents it to show POD that it is code.

sub xtidy
{
    my ($text) = @_;

    # Remove shebang.

    $text =~ s/^#!.*$//m;

    # Remove sobvious.

    $text =~ s/use\s+(strict|warnings);\s+//g;
    $text =~ s/^binmode\s+STDOUT.*?utf8.*?\s+$//gm;
    $text =~ s/use\s+JSON::Parse.*?;\s+//;

    # Replace tabs with spaces.

    $text =~ s/ {0,7}\t/        /g;

    # Add indentation.

    $text =~ s/^(.*)/    $1/gm;

    return $text;
}
