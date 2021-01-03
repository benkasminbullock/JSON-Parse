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
	my $mod = $mcpan->module ($module);
	my $dist = $mod->distribution ();
	my $fav = $mcpan->favorite ({distribution => $dist});
	my %info;
	$info{module} = $module;
	$info{version} = $mod->version ();
	$info{author} = $mod->author ();
	$info{date} = $mod->date ();
	$info{fav} = $fav->total ();
	push @modules, \%info;
    }
};
if ($@) {
    print "error: $@";
}
my $mtext = create_json (\@modules, sort => 1, indent => 1);
write_text ("see-also-info.json", $mtext);


