#!/home/ben/software/install/bin/perl

# This makes "errors.c" from "errormsg.txt" and "expectations.txt".

use warnings;
use strict;
use FindBin '$Bin';
use Table::Readable 'read_table';

# Input file, list of error messages like "Unexpected character".

my $in = 'errormsg.txt';
my @e = read_table ($in);
my @strings;
my @labels;

# Output file.

my $out = "$Bin/errors.c";
if (-f $out) {
    chmod 0644, $out or die $!;
}
open my $o, ">", $out or die $!;
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

# Print the enum of various types of error.

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

# Print the stringified errors.

for my $printed (@strings) {
    print $o "    \"$printed\",\n";
}
print $o <<EOF;
};
EOF

# Get the table of expected bytes.

my @expectations = read_table ('expectations.txt');

# The definitions of macros which go into "parser->expected". These
# all start with an "X".

my @us;

# The name of the expectation.

my @exs;

# The descriptions.

my @ds;

# Arrays of valid/invalid bytes.

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
	$r = "[$r]";
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

# Make an enum of expectations, although these are not used very much
# in the program.

print $o "enum expectation {\n";
for my $c (@exs) {
    print $o "    x$c,\n";
}
print $o "    n_expectations\n};\n";

# Print the definitions.

for my $u (@us) {
    print $o "$u\n";
}

# Print the expectation messages.

print $o "char * input_expectation[n_expectations] = {\n";
for my $d (@ds) {
    print $o "\"$d\",\n";
}
print $o "};\n";

# Print all of the expectations as one giant array.

print $o "unsigned char allowed[n_expectations][JSON3MAXBYTE] = {\n";
for my $i (0..$#arrays) {
    print $o "/* $exs[$i] */\n";
    print $o "$arrays[$i]\n";
}
print $o "};\n";

close $o or die $!;
chmod 0444, $out or die $!;
exit;
