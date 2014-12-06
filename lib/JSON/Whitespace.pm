package JSON::Whitespace;
use parent Exporter;
our @EXPORT_OK = qw/json_no_space/;
use warnings;
use strict;
use Carp;

use JSON::Tokenize 'tokenize_json';

sub json_no_space
{
    my ($json) = @_;
    my $tokens = tokenize_json ($json);
    my $nospace = strip_whitespace ($tokens, $json);
    return $nospace;
}

# my @values = (qw/
# array_indent
# object_indent
# before_comma
# after_comma
# before_colon
# after_colon
# before_object_start
# after_object_start
# before_array_start
# after_array_start
# before_literal
# after_literal
# before_number
# after_number
# /
# );

# my %whitespace = (
# object_indent => "\t",
# array_indent => "\t",
# all => [' ', ' '],
# literal => [' ', ' '],
# ':' => ["\t" x 2, "\n"],
# '{' => ["\t" x 1, "\n"],
# ',' => ["  ", "\n"],
# openclose # controls [,{,},]
# indent # controls array_indent, object_indent
# );

# my $json = json_whitespace ($json, %whitespace);

# my $json = json_no_space ($json);
# json_whitespace ($json, all => ['', '']);
# my $json = json_readable ($json);
# json_whitespace (
#     $json,
#     ':' => ['', ' '],
#     all => ['', ''],
#     open => ['', "\n"],
#     close => ["\n", "\n"],
# );

1;
