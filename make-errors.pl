#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Table::Readable 'read_table';
my $in = 'errormsg.txt';
my @e = read_table ($in);
my @strings;
my @labels;
my $out = 'errors.c';
open my $o, ">", $out or die $!;
#select $o;
for my $e (@e) {
    my $error = $e->{error};
    if ($error) {
	my $printed = ucfirst ($e->{error});
	my $label = $error;
	$label =~ s/\s*'%c'$//;
	$label =~ s/\s/_/g;
	push @strings, $printed;
	push @labels, $label;
    }
}

print $o <<EOF;
typedef enum {
EOF
for my $label (@labels) {
    print $o "    json_error_$label,\n";
}
print $o <<EOF;
    json_error_overflow
}
json_error_t;

const char * json_errors[json_error_overflow] = {
EOF
for my $printed (@strings) {
    print $o "    \"$printed\",\n";
}
print $o <<EOF;
};
EOF

my @expectations = read_table ('expectations.txt');

my @us;
my @exs;
my @ds;
my @arrays;
my %regexes;
my $perline = 32;
for my $expectation (@expectations) {
    my $c = $expectation->{category};
    push @exs, $c;
    my $u = 'X' . uc $c;
    push @us, "#define $u (1<<x$c)";
    my $d = $expectation->{description};
    my $chrs = $expectation->{chars};
    if ($chrs) {
	my @chrs = split /\s/, $chrs;
	$d .= ": " . join ', ', (map {"'$_'"} @chrs);
	$d =~ s/\\s/ /g;
	$d =~ s/\\/\\\\/g;
	$d =~ s/"/\\"/g;
	my $r = '';
	for my $chr (@chrs) {
	    if ($chr eq '\s') {
		$r .= ' ';
	    }
	    elsif ($chr =~ /\\|-/) {
		$r .= $chr;
	    }
	    else {
		$r .= quotemeta ($chr);
	    }
	}
#	$r = quotemeta ($r);
	$r = "[$r]";
#	print "<@chrs> $r\n";
	$regexes{$c} = $r;
    }
    else {
	$regexes{$c} = '[^\x00-\xFF]';
    }
    my $r = $regexes{$c};
    my $array = "{";
    for (0..255) {
	if (chr ($_) =~ /$r/) {
	    $array .= "1,";
	}
	else {
	    $array .= "0,";
	}
	if ($_ % $perline == $perline -1) {
	    $array .= "\n ";
	}
    }
    $array .= "},";
    push @arrays, $array;
    push @ds, $d;
}

print $o "enum expectation {\n";
for my $c (@exs) {
    print $o "    x$c,\n";
}
print $o "    n_expectations\n};\n";

for my $u (@us) {
    print $o "$u\n";
}

print $o "char * input_expectation[n_expectations] = {\n";
for my $d (@ds) {
    print $o "\"$d\",\n";
}
print $o "};\n";

print $o "unsigned char allowed[n_expectations][MAXBYTE] = {\n";
for my $i (0..$#arrays) {
    print $o "/* $exs[$i] */\n";
    print $o "$arrays[$i]\n";
}
print $o "};\n";

close $o or die $!;

exit;
