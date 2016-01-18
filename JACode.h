#pragma once

namespace libeburc
{
	class JACode
	{
		JACode();
	public:
		static void eb_jisx0208_to_euc( char *out_string, const char *in_string );
	};
}
