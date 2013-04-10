#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "Json3-perl.c"

MODULE=JSON::Parse PACKAGE=JSON::Parse

PROTOTYPES: ENABLE

BOOT:
{
	json_true = get_sv ("JSON::Parse::true", GV_ADD);
	json_false = get_sv ("JSON::Parse::false", GV_ADD);
	json_null = get_sv ("JSON::Parse::null", GV_ADD);
}

SV * parse_json (json)
	SV * json;
CODE:
	RETVAL = parse (json);
OUTPUT:
	RETVAL
