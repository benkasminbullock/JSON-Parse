#!/home/ben/software/install/bin/perl
use warnings;
use strict;

# http://cpansearch.perl.org/src/MARKF/Test-utf8-1.00/lib/Test/utf8.pm

# https://rt.cpan.org/Ticket/Display.html?id=91537

# http://perldoc.perl.org/perlunicode.html#Unicode-Encodings

our $utf8_re = <<'REGEX' ;
        [\x{00}-\x{7f}]
      | [\x{c2}-\x{df}][\x{80}-\x{bf}]
      |         \x{e0} [\x{a0}-\x{bf}][\x{80}-\x{bf}]
      | [\x{e1}-\x{ec}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{ed} [\x{80}-\x{9f}][\x{80}-\x{bf}]
      | [\x{ee}-\x{ef}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{f0} [\x{90}-\x{bf}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      | [\x{f1}-\x{f3}][\x{80}-\x{bf}][\x{80}-\x{bf}][\x{80}-\x{bf}]
      |         \x{f4} [\x{80}-\x{8f}][\x{80}-\x{bf}][\x{80}-\x{bf}]
REGEX

my %ranges;

my $h = qr/([0-9a-f]{2})/;
my $hex = qr/\\x\{$h\}/i;

my $cfile = 'utf8.c';
my $expected = 'utf8-expected.txt';

open my $out, ">", $cfile or die $!;

open my $eout, ">", $expected or die $!;

while ($utf8_re =~ /\[$hex-$hex\]/g) {
    $ranges{"$1$2"} = 1;
}
for my $range (sort {hex $a <=> hex $b} keys %ranges) {
    my ($lower, $upper) = ($range =~ /$h/g);
    my $macro = "BYTE_" . uc ($lower) . "_" . uc ($upper);
    print $out "#define $macro \\\n";
    my $l = hex $lower;
    my $u = hex $upper;
    printf $out "      0x%02X:", $l;
    for ($l + 1..$u - 1) {
	printf $out " case 0x%02X:", $_;
	if ((($_ - $l) % 7) == 6) {
	    print $out "\\\n";
	}
    }
    printf $out " case 0x%02X\n", $u;
    my $notrange = lc ("not_${lower}_$upper");
    printf $eout <<EOF;
category: $notrange
description: bytes in range $lower-$upper
chars: \\x$lower-\\x$upper

EOF
}
close $eout or die $!;

my $switch = <<EOF;
switch (NEXTBYTE) {
case BYTE_00_7F:
    if (c == '\\n') {
        line++;
    }
    goto string_start;

case BYTE_C2_DF:
    goto byte_last_80_bf;

case 0xE0:
    goto byte23_a0_bf;

case BYTE_E1_EC:
    goto byte_penultimate_80_bf;

case 0xED:
    goto byte23_80_9f;

case BYTE_EE_EF:
    goto byte_penultimate_80_bf;

case 0xF0:
    goto byte24_90_bf;

case BYTE_F1_F3:
    goto byte24_80_bf;

case 0xF4:
    goto byte24_80_8f;

default:
    FAILUTF8;
}

byte_last_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto string_start;
default:
    FAILUTF8;
}

byte_penultimate_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte24_90_bf:

switch (NEXTBYTE) {

case BYTE_90_BF:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}

byte23_80_9f:

switch (NEXTBYTE) {

case BYTE_80_9F:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte23_a0_bf:

switch (NEXTBYTE) {

case BYTE_A0_BF:
    goto byte_last_80_bf;
default:
    FAILUTF8;
}

byte24_80_bf:

switch (NEXTBYTE) {

case BYTE_80_BF:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}

byte24_80_8f:

switch (NEXTBYTE) {

case BYTE_80_8F:
    goto byte_penultimate_80_bf;
default:
    FAILUTF8;
}
EOF
$switch =~ s/(^|\n)(.*)/$1    $2/g;
my $stuff =<<'EOF';
EOF
$stuff .= $switch;
$stuff .= <<'EOF';
EOF

print $out "$stuff\n";
close $out or die $!;
exit;


