#pragma once
#include <pch.h>
#include <eburc/defs.h>
#include <eburc/build-post.h>

using namespace Platform;

using eb_map = unordered_map<int, const char*>;
using eb_msize = unordered_map<int, int>;

namespace libeburc
{
	ref class EBMap sealed
	{
	internal:
		int character_number;
		eb_map conv_map;
		eb_msize conv_size;

		int open_index;
		bool feed_any = false;

		const char* char_expecting;
		const char* exp_fhead;

		void Feed( const char* b );

		const char* Get( int index );
		int Size( int index );

		int bytes_needed = 0;
		int bytes_filled = 0;
		char buff[ 4 ];

		int bytes_feeded = 0;
		char bbuff[ 64 ];

		EBMap( EBMapType type );
	public:
	};
}
