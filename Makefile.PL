use strict;
use warnings;
use ExtUtils::MakeMaker;

my $pm = 'lib/JSON/Parse.pm';
my $pod = 'lib/JSON/Parse.pod';
my $repo = 'https://github.com/benkasminbullock/JSON-Parse';

WriteMakefile (
    NAME => 'JSON::Parse',
    VERSION_FROM => $pm,
    ABSTRACT_FROM => $pod,
    AUTHOR => 'Ben Bullock <bkb@cpan.org>',
    LICENSE => 'perl',
    PREREQ_PM => {
    },
    META_MERGE => {
        resources => {
            repository => $repo,
            bugtracker => "$repo/issues",
        },
    },
    OBJECT => 'Json3.o',
    # Currently the oldest version to pass at CPAN testers.
    # http://matrix.cpantesters.org/?dist=JSON-Parse+0.24
    MIN_PERL_VERSION => '5.008009',

#    OPTIMIZE => '-Wall',
#    OPTIMIZE => '-O3',
);
