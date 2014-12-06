# This does only one job, exporting the single function below.

package JSON::Tokenize;
use JSON::Parse;
use parent Exporter;
our @EXPORT_OK = qw/tokenize_json/;
use warnings;
use strict;
use Carp;

1;
