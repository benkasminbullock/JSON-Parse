package JPXT;
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw/daft_test/;
our %EXPORT_TAGS = (all => \@EXPORT_OK);
use warnings;
use strict;
use utf8;
use Carp;
use Test::More;

sub daft_test
{
    my ($y) = @_;
    if ($y =~ /FFF[EF]/ || $y =~ /FDD0/ || $y =~ /surrogates/ ||
	$y =~ /noncharacter/) {
	diag ("Skipping utterly daft test '$y'");
	return 1;
    }
    return undef;
}

1;
