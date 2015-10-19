#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Deploy 'file_slurp';
use ExtUtils::testlib;
use Test::More;
use JSON::Whitespace 'json_no_space';
my $json = <<'EOF';
{
   "stat" : {
      "uid" : 500,
      "mtime" : 1316754239,
      "mode" : 33188,
      "size" : 7715,
      "gid" : 500
   },
   "level" : 2,
   "sloc" : 143,
   "slop" : 94,
   "mime" : "text/x-script.perl-module"
}
EOF

is (json_no_space ($json), '{"stat":{"uid":500,"mtime":1316754239,"mode":33188,"size":7715,"gid":500},"level":2,"sloc":143,"slop":94,"mime":"text/x-script.perl-module"}');
done_testing ();
