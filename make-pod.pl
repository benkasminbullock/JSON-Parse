#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Template;
use FindBin '$Bin';
use lib 'blib/arch';
use lib 'blib/lib';
use JSON::Parse 'assert_valid_json';
use Table::Readable 'read_table';
use Perl::Build::Pod ':all';
use Perl::Build qw/get_version get_commit/;

# Names of the input and output files containing the documentation.

my $pod = 'Parse.pod';
my $input = "$Bin/lib/JSON/$pod.tmpl";
my $output = "$Bin/lib/JSON/$pod";

# Template toolkit variable holder

my %vars;

$vars{version} = get_version ();
$vars{commit} = get_commit ();

my $tt = Template->new (
    ABSOLUTE => 1,
    INCLUDE_PATH => [
	$Bin,
	pbtmpl (),
	"$Bin/examples",
    ],
    ENCODING => 'UTF8',
    FILTERS => {
        xtidy => [
            \& xtidy,
            0,
        ],
    },
    STRICT => 1,
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
	my $expanded;
	my $test;
	if ($error->{error} =~ /not unique/) {
	    $test = <<EOF;
    my \$p = JSON::Parse->new ();
    \$p->detect_collisions (1);
    \$p->run ('$example');
EOF
	}
	else {
	    $test = <<EOF;
    assert_valid_json ('$example');
EOF
	}
	eval "$test";
	if (! $@) {
	    warn "No error with $example running $error->{error}";
	}
	my $out = $@;
	if ($out !~ /\Q$error->{error}/i) {
	    # Don't die here since if module is faulty this can
	    # happen, then a circular problem occurs.
	    warn "Did not get $error->{error} parsing $example";
	}
	# Remove this file's name from the error message.
	$out =~ s/at \(eval.*|\.\/make-pod\.pl.*$//;
	$expanded = <<EOF;
$test

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
    $tt->process (\$d, \%vars, \my $out, binmode => 'utf8')
        or die '' . $tt->error ();
    $error->{description}  = $out;
#    print "$error->{description}\n";
    $error->{error} = ucfirst ($error->{error});
}
$vars{errors} = \@errors;
$tt->process ($input, \%vars, $output, binmode => 'utf8')
    or die '' . $tt->error ();

exit;

