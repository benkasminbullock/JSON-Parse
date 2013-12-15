#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "unicode.h"
#include "unicode.c"
#include "Json3-perl-common.c"
#define PERLING
#include "Json3-perl.c"
#undef PERLING
#include "Json3-perl.c"
#include "Json3-entry-points.c"

MODULE=JSON::Parse PACKAGE=JSON::Parse

PROTOTYPES: ENABLE

BOOT:
{
	json_true = get_sv ("JSON::Parse::true", GV_ADD);
	sv_setiv (json_true, 1);
	SvREADONLY_on (json_true);
	json_false = get_sv ("JSON::Parse::false", GV_ADD);
	sv_setiv (json_false, 0);
	SvREADONLY_on (json_false);
	json_null = get_sv ("JSON::Parse::null", GV_ADD);
	SvREADONLY_on (json_null);
}

SV * parse_json (json)
	SV * json;
CODE:
	RETVAL = parse (json);
OUTPUT:
	RETVAL

void assert_valid_json (json)
	SV * json;
CODE:
	validate (json);
