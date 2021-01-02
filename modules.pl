#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use utf8;
use FindBin '$Bin';
use File::Slurper 'read_text', 'write_text';
use Test::More;
use MetaCPAN::Client;
use JSON::Create 'create_json';
use lib "$Bin/build";
use JPB;

# Read the file in & extract the section

my $text = see_also ();

my $mcpan = MetaCPAN::Client->new ();

my @modules;

eval {
    while ($text =~ /$mod_re/g) {
	my $module = $1;
	print "$module\n";
#	next;
	my $dist = $mcpan->module ($module);
	my %info;
	$info{module} = $module;
	$info{version} = $dist->version ();
	$info{author} = $dist->author ();
	$info{date} = $dist->date ();
	push @modules, \%info;
    }
};
if ($@) {
    print "error: $@";
}
my $mtext = create_json (\@modules, sort => 1, indent => 1);
write_text ("see-also-info.json", $mtext);


