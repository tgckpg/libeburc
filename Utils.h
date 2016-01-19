#pragma once

#include <defs.h>

using namespace std;

namespace libeburc
{
	class Utils
	{
		Utils();
	public:
		static wstring ToWStr( const char * c );
		static byte* MBEUCJP16( const char * c );
	};
}
