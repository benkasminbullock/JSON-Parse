#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Perl::Build;
perl_build (
    c => [{
	dir => '/home/ben/projects/unicode-c',
	stems => ['unicode'],
    },],
    pod => [
	'lib/JSON/Parse.pod',
    ],
);
exit;
