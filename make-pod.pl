#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Template;
use FindBin;
use lib 'blib/arch';
use lib 'blib/lib';
use JSON::Parse 'validate_json';
use Table::Readable 'read_table';

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

my @errors = read_table ('errormsg.txt');
# Remove "invalid".

shift @errors;

for my $error (@errors) {
    my $d = $error->{description};
    my $count = 0;
    while ($d =~ /(!(.*?)!)/) {
	my $text = $1;
	my $example = $2;
	die if !$example;
	eval {
	    validate_json ($example);
	};
	if (! $@) {
	    warn "No error running $error";
	}
	my $out = $@;
	if ($out !~ /\Q$error->{error}/i) {
	    # Don't die here since if module is faulty this can
	    # happen, then a circular problem occurs.
	    warn "Did not get $error->{error} parsing $example";
	}
	# Remove this file's name from the error message.
	$out =~ s/at \.\/make-pod\.pl.*$//;
	my $expanded = <<EOF;
    validate_json ('$example');

gives output

    $out
EOF
	$d =~ s/\Q$text/$expanded/;
	$count++;
	if ($count > 19) {
	    die "$count over";
	}
    }
#    print "$d\n";
    $tt->process (\$d, \%vars, \my $out, {binmode => 'utf8'})
        or die '' . $tt->error ();
    $error->{description}  = $out;
#    print "$error->{description}\n";
    $error->{error} = ucfirst ($error->{error});
}
$vars{errors} = \@errors;
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
#    $text =~ s/use\s+JSON::Parse.*?;\s+//;

    # Replace tabs with spaces.

    $text =~ s/ {0,7}\t/        /g;

    # Add indentation.

    $text =~ s/^(.*)/    $1/gm;

    return $text;
}
