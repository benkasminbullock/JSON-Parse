#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Template;
use FindBin '$Bin';
use lib 'blib/arch';
use lib 'blib/lib';
use JSON::Parse qw!json_file_to_perl assert_valid_json!;
use Table::Readable 'read_table';
use lib "$Bin/copied/lib";
use Perl::Build::Pod ':all';
use Perl::Build qw/get_version get_commit get_info/;

make_examples ("$Bin/examples", undef, undef);

# Template toolkit variable holder

my %vars;

my $info = get_info (base => $Bin,);
$vars{info} = $info;
$vars{version} = get_version ();
$vars{commit} = get_commit ();
my $see_also_info = json_file_to_perl ("$Bin/see-also-info.json");
my %mod2info;
for (@$see_also_info) {
    $_->{date} =~ s/T.*$//;
    $_->{log_fav} = 0;
    if ($_->{fav} > 0) {
	$_->{log_fav} = int (log ($_->{fav}) / log (10)) + 1;
    }
    $mod2info{$_->{module}} = $_;
}
$vars{mod2info} = \%mod2info;
$vars{benchv} = json_file_to_perl ("$Bin/benchmarks/bench-versions.json");

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
$vars{errors} = read_errors ($tt, \%vars);

# Names of the input and output files containing the documentation.

for my $mod (qw/Parse Tokenize/) {
    my $version = <<EOF;
=head1 VERSION

This documents version [% version %] of JSON::$mod corresponding to
L<git commit [% commit.commit
%]|[% info.repo %]/commit/[%
commit.commit %]> released on [% commit.date %].

EOF
    $tt->process (\$version, \%vars, \my $tt_version, binmode => ':encoding(utf8)')
        or die $tt->error () . '';
    $vars{version_text} = $tt_version;
    my $pod = "$mod.pod";
    my $input = "$Bin/lib/JSON/$pod.tmpl";
    my $output = "$Bin/lib/JSON/$pod";
    $tt->process ($input, \%vars, $output, binmode => 'utf8')
        or die '' . $tt->error ();
}
exit;

sub read_errors
{
    my ($tt, $vars) = @_;
    my @errors = read_table ('errormsg.txt');
    # Remove "invalid".

    shift @errors;

    for my $error (@errors) {
	my $d = $error->{description};
	my $count = 0;
	while ($d =~ /(!(.*?)!)/) {
	    my $text = $1;
	    my $example = $2;
#	    die "No example" if !$example;
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
	$tt->process (\$d, $vars, \my $out, binmode => 'utf8')
        or die '' . $tt->error ();
	$error->{description}  = $out;
	#    print "$error->{description}\n";
	$error->{error} = ucfirst ($error->{error});
    }
    return \@errors;
}
