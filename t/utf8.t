use warnings;
use strict;
use Test::More;
use JSON::Parse ':all';

# Test valid JSON.

no utf8;

ok (valid_json ('["蟹"]'));
ok (valid_json ('{"動物":"像"}'));
my $bad_cont = sprintf ('["%c"]', 0x80);
ok (! valid_json ($bad_cont));
eval {
    assert_valid_json ($bad_cont);
};
like ($@, qr/Unexpected character 0x80 parsing string/);


use utf8;

ok (valid_json ('["蟹"]'));
ok (valid_json ('{"動物":"像"}'));

done_testing ();
exit;
