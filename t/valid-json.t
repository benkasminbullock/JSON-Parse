use warnings;
use strict;
use Test::More;
use JSON::Parse qw/valid_json/;

# https://github.com/benkasminbullock/JSON-Parse/issues/2

my $fdegir1 = <<'EOF';
{
"gav": {
"groupId": "mygroup",
"artifactId": "myartifact"
"version": "1.0"
}
}
EOF

ok (! valid_json ($fdegir1));

TODO: {
    local $TODO = 'known bugs';
};
done_testing ();
