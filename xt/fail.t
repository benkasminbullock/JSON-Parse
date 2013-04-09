use warnings;
use strict;
use Test::More;
use Json3 'parse_json';

# Test of fault-finding.

failit ('["a":1]', qr/unknown character.*':'/);
failit ('{1,2,3}', qr/unparseable character '1' in object/i);
failit ('[1,2,3}', qr/unknown character.*'}'/i);
failit ('["\z"]', qr/unknown escape \\z/i);
failit ('{"go":{"buddy":{"go":{"buddy":', qr/null byte in string/);
failit ('{"gobuggs}', qr/unknown character .* in object/i);

done_testing ();

sub failit
{
    my ($input, $expect) = @_;
    eval {
	my $j = parse_json ($input);
    };
    ok ($@, "Got error for $input");
    like ($@, $expect, "Got correct-looking error $expect for $input");
}
