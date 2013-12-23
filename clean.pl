#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Deploy 'do_system';
unlink (qw!errors.c README!);
do_system ("make -f randmakefile clean");
