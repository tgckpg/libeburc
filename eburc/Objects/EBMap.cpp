#include "pch.h"
#include "eburc/Objects/EBMap.h"

using namespace libeburc;

const char * exp_shead = "un-";
const char * exp_lf = "\n";
const char * exp_hex = "0123456789ABCDEF";
const char * exp_tab = "\t ";

const char * trim_euc_utf16( char *buff, int l, int *out_size )
{
	bool tabstart = true;

	int i = 0;

	// Fit [ i, l ] to the content
	for ( ; i < l; i++ )
	{
		char c = buff[ i ];
		if ( c == '\t' || c == ' ' || c == '\r' ) continue;
		break;
	}

	for ( ; i < l; l-- )
	{
		char c = buff[ l - 1 ];
		if ( c == '\t' || c == ' ' || c == '\r' ) continue;
		break;
	}

	if ( i == l ) return nullptr;

	*out_size = MultiByteToWideChar( CP_SHIFT_JIS, 0, ( LPCSTR ) ( buff + i ), l - i, NULL, 0 );
	*out_size *= 2;

	char *ubuff = new char[ *out_size ];

	MultiByteToWideChar( CP_SHIFT_JIS, 0, ( LPCSTR ) ( buff + i ), -1, ( LPWSTR ) ubuff, *out_size );

	return ubuff;
}

void EBMap::Feed( const char* b )
{
	if ( feed_any && open_index )
	{
		if ( *b == '\n' || *b == '#' )
		{
			int *j = new int( 0 );
			const char * t16map = trim_euc_utf16( bbuff, bytes_feeded, j );

			if ( t16map != nullptr )
			{
				conv_map.insert( eb_map::value_type( open_index, t16map ) );
				conv_size.insert( eb_msize::value_type( open_index, *j ) );
			}
			else
			{
				delete j;
			}

			open_index = 0;
			feed_any = false;

			char_expecting = exp_fhead;
		}
		else
		{
			bbuff[ bytes_feeded++ ] = *b;
		}

		return;
	}

	else if ( *b == '\n' )
	{
		bytes_needed = 0;
		bytes_filled = 0;
		open_index = 0;

		char_expecting = exp_fhead;
		return;
	}

	else if ( *b == '#' )
	{
		char_expecting = exp_lf;
		return;
	}

	if ( strpbrk( char_expecting, b ) == 0 ) return;

	if ( char_expecting == exp_fhead )
	{
		bytes_filled = 0;
		bytes_needed = 4;

		char_expecting = exp_hex;
	}

	else if ( char_expecting == exp_shead )
	{
		if ( *b == '-' )
		{
			feed_any = true;
			bytes_feeded = 0;

			char_expecting = exp_lf;
		}
		else if ( *b == 'n' )
		{
			open_index = 0;
			char_expecting = exp_lf;
		}
		else if ( *b == 'u' )
		{
			bytes_filled = 0;
			bytes_needed = 4;

			char_expecting = exp_hex;
		}
	}

	else if ( char_expecting == exp_hex )
	{
		if ( bytes_filled < bytes_needed )
		{
			buff[ bytes_filled++ ] = *b;

			if ( bytes_filled == bytes_needed )
			{
				char* ab = new char[ 2 ];
				ab[ 0 ] = eb_hexbyte( buff );
				ab[ 1 ] = eb_hexbyte( ( buff + 2 ) );

				if ( open_index == 0 )
				{
					open_index = eb_uint2( ab );
					char_expecting = exp_shead;

					delete ab;
				}
				else
				{
					swap( ab[0], ab[1] );

					conv_map.insert( eb_map::value_type( open_index, ab ) );
					conv_size.insert( eb_msize::value_type( open_index, 2 ) );
					open_index = 0;
					char_expecting = exp_lf;
				}
			}
		}
	}

}

const char* EBMap::Get( int index )
{
	if( conv_map.count( index ) )
		return conv_map.at( index );
	return nullptr;
}

int EBMap::Size( int index )
{
	return conv_size.at( index );
}

EBMap::EBMap( EBMapType type )
{
	switch ( type )
	{
	case EBMapType::EB_MAP_C:
		exp_fhead = "#c";
		break;
	case EBMapType::EB_MAP_G:
		exp_fhead = "#g";
		break;
	case EBMapType::EB_MAP_HAN_FONT:
		exp_fhead = "#h";
		break;
	case EBMapType::EB_MAP_ZEN_FONT:
		exp_fhead = "#z";
		break;
	}

	char_expecting = exp_fhead;
}
