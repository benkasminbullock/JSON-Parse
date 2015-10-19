#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use 5.010;
 
use JSON::Create;
use JSON::PP ();
use JSON::XS ();
use Cpanel::JSON::XS ();
use JSON::Parse ();
use Benchmark qw(cmpthese);
 
my $json = create_json(100);
cmpthese(10000, {
    'JSON'     => sub { JSON::PP::decode_json($json) },
    'JSON::XS' => sub { JSON::XS::decode_json($json) },
    'Cpanel::JSON::XS' => sub { Cpanel::JSON::XS::decode_json($json) },
    'JSON::Parse' => sub { JSON::Parse::parse_json ($json) },
});
 
sub create_json {
    my ($n) = @_;
    my @data = map { { $_ => $_ } } 1 .. $n;
    return JSON::Create::create_json \@data;
}
