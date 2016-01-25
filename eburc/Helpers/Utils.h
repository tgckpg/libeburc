#pragma once

#include <eburc/defs.h>

using namespace std;

namespace libeburc
{
	class Utils
	{
		Utils();
	public:
		static wstring ToWStr( const char* c );
		static byte* EucJP2Utf16( const char* c );
		static byte* Utf82EucJP( const wchar_t* c );
		static int ReadUtf8( const char* buffer, int* code );

		/*
		 * Get a BCD (binary coded decimal) packed integer with 2/4/6 bytes
		 * from an octet stream.
		 */
		static unsigned eb_bcd2( const char *stream );
		static unsigned eb_bcd4( const char *stream );
		static unsigned eb_bcd6( const char *stream );

	};
}
