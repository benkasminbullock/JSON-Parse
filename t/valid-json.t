use warnings;
use strict;
use Test::More;
use JSON::Parse qw/valid_json validate_json/;

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
eval {
    validate_json ($fdegir1);
};
ok ($@, "validate_json dies");
like ($@, qr/line 5/i, "line number OK");

my $empty = '  ';
ok (! valid_json ($empty));
eval {
    validate_json ($empty);
};
ok ($@, "empty input dies");
like ($@, qr/empty input/i, "flagged as empty input");

my $undef = undef;
ok (! valid_json ($undef));
eval {
    no warnings 'uninitialized';
    validate_json ($undef);
    use warnings 'uninitialized';
};
ok ($@, "undef input dies");
like ($@, qr/empty input/i, "flagged as empty input");

TODO: {
    local $TODO = 'known bugs';
};
done_testing ();
