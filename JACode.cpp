#include "pch.h"
#include "JACode.h"

using namespace libeburc;

JACode::JACode()
{
}

void JACode::eb_jisx0208_to_euc( char *out_string, const char *in_string )
{
	unsigned char *out_p = ( unsigned char * ) out_string;
	const unsigned char *in_p = ( unsigned char * ) in_string;

	while ( *in_p != '\0' )
		*out_p++ = ( ( *in_p++ ) | 0x80 );

	*out_p = '\0';
}