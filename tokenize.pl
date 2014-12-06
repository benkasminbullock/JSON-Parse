#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use ExtUtils::testlib;
use JSON::Tokenize 'tokenize_json';
use Deploy 'file_slurp';
#JSON::Tokenize::tokenize ('{"tuttie":"fruity"}');
tokenize_json ('{"a":"b","c":"d"}');
tokenize_json ('{"tuttie":["fruity", true, 100]}');
tokenize_json ('["fruity", true, 100]');
my $cpantesters = file_slurp ('benchmarks/cpantesters.json');
#JSON::Tokenize::tokenize ($cpantesters);
