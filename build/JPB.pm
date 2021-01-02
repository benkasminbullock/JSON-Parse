package JPB; # Json Parse Build
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw/
		    $mod_re
		    see_also
		/;
use warnings;
use strict;
use utf8;
use Carp;
use File::Slurper 'read_text';

my $dir = __FILE__;
$dir =~ s!/[^/]*$!!;
die unless -d $dir;

our $mod_re = qr!\[\%\s*cpm\(['"](.*?)['"]\)\s*\%\]!;

sub see_also
{
    my $pod = "$dir/../lib/JSON/Parse.pod.tmpl";
    die unless -f $pod;
    my $text = read_text ($pod);
    if (! ($text =~ s!^.*=head1 SEE ALSO(.*?)=head1.*$!$1!gsm)) {
	die "Could not extract see also";
    }
    return $text;
}

1;
