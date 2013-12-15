use warnings;
use strict;
use JSON::Parse 'json_to_perl';
use Test::More;

my $p;

# This was causing some problems with the new grammar / lexer.

my $jeplus = '[1.9e+9]';
eval {
    $p = json_to_perl ($jeplus);
};
note ($@);
ok (! $@, "Parsed OK");

my $j = <<EOF;
{
   "integer":100,
   "decimal":1.5,
   "fraction":0.01,
   "exponent-":1.9e-2,
   "exponent+":1.9e+9,
   "exponent":1.0E2
}
EOF
eval {
    $p = json_to_perl ($j);
};
note ($@);
ok (! $@, "Parsed OK");
if ($@) {
    print $@;
    exit;
}
ok (compare ($p->{integer}, 100));
ok (compare ($p->{decimal} , 1.5));
ok (compare ($p->{exponent} , 100));
ok (compare ($p->{"exponent-"} , 19/1000));
ok (compare ($p->{"exponent+"} , 1_900_000_000));
ok (compare ($p->{fraction} , 0.01));
my $q = @{json_to_perl ('[0.12345]')}[0];
ok (compare ($q, '0.12345'));
# Illegal numbers

eval {
    json_to_perl ('[0...111]');
};
ok ($@);
eval {
    json_to_perl ('[0111]');
};
like ($@, qr/leading zero/i, "Error for leading zero");


done_testing;
exit;

sub compare
{
    my ($x, $y) = @_;
    my $error = 0.00001;
    if (abs ($x - $y) < $error) {
        return 1;
    }
    print "$x and $y are not equal.\n";
    return;
}
