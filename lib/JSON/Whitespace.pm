package JSON::Whitespace;
use warnings;
use strict;
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw/json_no_space/;
our %EXPORT_TAGS = (
    all => \@EXPORT_OK,
);

use warnings;
use strict;
use Carp;
our $VERSION = '0.59_01';

use JSON::Tokenize 'tokenize_json';

sub json_no_space
{
    my ($json) = @_;
    my $tokens = tokenize_json ($json);
    my $nospace = strip_whitespace ($tokens, $json);
    return $nospace;
}

1;
