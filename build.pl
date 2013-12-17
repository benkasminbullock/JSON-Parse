#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Perl::Build;
perl_build (
    c => [{
	dir => '/home/ben/projects/unicode-c',
	stems => ['unicode'],
    },],
    make_pod => './make-pod.pl',
    clean => './clean.pl',
    pre => './make-errors.pl',
);
exit;
