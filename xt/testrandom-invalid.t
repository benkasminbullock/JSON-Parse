use warnings;
use strict;
use Test::More;
use JSON::Parse;

# Make sure that the random json generator stuff is all switched OFF
# for the production version.

eval {
    JSON::Parse::random_json ();
};
ok ($@, "got error running random_json ()");
like ($@, qr/Undefined subroutine/, "random_json () is made invalid");
done_testing ();

