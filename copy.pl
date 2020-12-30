#!/home/ben/software/install/bin/perl

# The build process of this module relies on lots of modules which are
# not on CPAN. To make it possible for people to use the repository
# from github, and in order to allow continuous integration, copy the
# local modules into the git repo.

use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use Sys::Hostname;
use File::Slurper 'read_text';
use File::Copy;

use Deploy qw!copy_those_files do_system!;
my $host = hostname ();
if ($host ne 'mikan') {
    exit;
}

my $proj = '/home/ben/projects';
my $pbdir = "$proj/perl-build";
my $ddir = "$proj/deploy";
my $cmdir = "$proj/c-maker";
my $pdir = "$proj/purge";
my $udir = "$proj/unicode-c";
my @libs = ($pbdir, $ddir, $cmdir, $pdir);
my $copied = "$Bin/copied";
my $lib = "$copied/lib";
my $verbose;
#my $verbose = 1;
if ($verbose) {
    warn "Verbose messages are on";
}

# This is not a smart thing to do, if $lib happens to contain a typo
# we may end up using rm from $HOME, but it is necessary in some
# cases.

do_system ("cd $copied || exit;rm -rf ./lib", $verbose);
do_system ("mkdir -p $lib", $verbose);
for my $dir (@libs) {
    die unless -d $dir;
    copy_those_files ($dir, $lib, $verbose);
}

my $cudir = "$copied/unicode";
if (! -d $cudir) {
    do_system ("mkdir -p  $cudir");
}
for my $e (qw!c h!) {
    copy ("$udir/unicode.$e", "$cudir/unicode.$e");
}

#do_system ("git add $lib; git commit -m 'copied files'", $verbose);
exit;

