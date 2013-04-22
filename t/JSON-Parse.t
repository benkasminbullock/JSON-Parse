use warnings;
use strict;
use Test::More;
use JSON::Parse qw/json_to_perl valid_json/;
use utf8;

#binmode STDOUT, ":utf8";
my $jason = <<'EOF';
{"bog":"log","frog":[1,2,3],"guff":{"x":"y","z":"monkey","t":[0,1,2.3,4,59999]}}
EOF
my $x = gub ($jason);
note ($x->{guff}->{t}->[2]);
cmp_ok (abs ($x->{guff}->{t}->[2] - 2.3), '<', 0.00001, "Two point three");

my $fleece = '{"凄い":"技", "tickle":"baby"}';
my $y = gub ($fleece);
ok ($y->{tickle} eq 'baby', "Don't tickle baby");
ok (valid_json ($fleece), "Valid OK JSON");

my $argonauts = '{"medea":{"magic":true,"nice":false}}';
my $z = gub ($argonauts);
ok ($z->{medea}->{magic}, "Magic, magic, you can do magic");
ok (! ($z->{medea}->{nice}), "Now that's not very nice.");
ok (valid_json ($argonauts), "Valid OK JSON");

# Test that empty inputs result in an undefined return value, and no
# error message.

#my $p = json_to_perl (undef);
#ok (! defined $p, "Undef returns undef");
my $Q = json_to_perl ('');
ok (! defined $Q, "Empty string returns undef");
ok (! valid_json (''), "! Valid empty string");
my $n;
eval {
    $n = '{"骪":"\u9aaa"';
    my $nar = json_to_perl ($n);
};
ok ($@, "found error");
#like ($@, qr/end of the file was reached/, "Error message OK");
ok (! valid_json ($n), "! Not valid missing end }");
TODO: {
    local $TODO = '\u not implemented';
    my $m = '{"骪":"\u9aaa"}';
    my $ar = gub ($m);
    ok (defined $ar, "Unicode \\uXXXX parsed");
    is ($ar->{骪}, '骪', "Unicode \\uXXXX parsed correctly");
    ok (valid_json ($m), "Valid good JSON");
};
my $bad1 = '"bad":"city"}';
$@ = undef;
eval {
    json_to_perl ($bad1);
};
ok ($@, "found error in '$bad1'");
#like ($@, qr/stray characters/, "Error message as expected");
my $notjson = 'this is not lexable';
$@ = undef;
eval {
    json_to_perl ($notjson);
};
ok ($@, "Got error message");
#like ($@, qr/stray characters/i, "unlexable message $@ OK");
ok (! valid_json ($notjson), "Not valid bad json");

my $wi =<<EOF;
{
     "firstName": "John",
     "lastName": "Smith",
     "age": 25,
     "address":
     {
         "streetAddress": "21 2nd Street",
         "city": "New York",
         "state": "NY",
         "postalCode": "10021"
     },
     "phoneNumber":
     [
         {
           "type": "home",
           "number": "212 555-1234"
         },
         {
           "type": "fax",
           "number": "646 555-4567"
         }
     ]
 }
EOF
my $xi = gub ($wi);
ok ($xi->{address}->{postalCode} eq '10021', "Test a value $xi->{address}->{postalCode}");
ok (valid_json ($wi), "Validate");

my $perl_a = json_to_perl ('["a", "b", "c"]');
ok (ref $perl_a eq 'ARRAY', "json array to perl array");
my $perl_b = json_to_perl ('{"a":1, "b":2}');
ok (ref $perl_b eq 'HASH', "json object to perl hash");

# The following tests use utf8::is_utf8, which is not available for
# Perl versions less than 5.006.

SKIP: {
    skip "Skip utf8 tests due to perl version number", 2 unless $] >= 5.008;
    use utf8;
    my $scorpion = '["蠍"]';
    my $p1 = json_to_perl ($scorpion);
    ok (utf8::is_utf8 ($p1->[0]), "UTF-8 survives");
    no utf8;
    my $ebi = '["蠍"]';
    my $p2 = json_to_perl ($ebi);
    ok (! utf8::is_utf8 ($p2->[0]), "Not UTF-8 not marked as UTF-8");
};


# See https://rt.cpan.org/Ticket/Display.html?id=73743
done_testing ();
exit;

sub gub
{
    my ($json) = @_;
#    print "Processing $json\n";
    my $p = json_to_perl ($json);
#    print "$p\n";
# Uncommend this to bugger things up
#    blub ($p);
    return $p;
}

sub blub
{
    my ($w, $indent) = @_;
    if (! defined $indent) {
        $indent = 0;
    }
    if (ref $w eq 'ARRAY') {
        indent ($indent, "[");
        for my $e (@$w) {
            blub ($e, $indent+1);
        }
        indent ($indent, "]");
    }
    elsif (ref $w eq 'HASH') {
        indent ($indent, "{");
        for my $k (keys %$w) {
            indent ($indent, "$k:");
            blub ($w->{$k}, $indent+1);
        }
        indent ($indent, "}");
    }
    else {
        indent ($indent, $w);
    }
}

sub indent
{
    my ($indent, $w) = @_;
    print "  " x $indent;
    print "$w\n";
}

# Local variables:
# mode: perl
# End:
