#include "pch.h"
#include "Utils.h"

using namespace libeburc;

Utils::Utils() { }

wstring Utils::ToWStr( const char * c )
{
	const size_t cSize = _mbstrlen(c) + 1;
	wstring wc( cSize, L'#' );
	size_t * nt = nullptr;
	mbstowcs_s( nt, &wc[0], cSize, c, cSize );

	return wc;
}

byte* Utils::MBEUCJP16( const char * c )
{
	LPCSTR pBuff = ( LPCSTR ) c;
	size_t Size = MultiByteToWideChar( CP_EUCJP, 0, pBuff, -1, NULL, 0 );

	byte* buff16 = new byte[ Size * 2 + 2 ];
	MultiByteToWideChar( CP_EUCJP, 0, pBuff, -1, ( LPWSTR ) buff16, Size );

	return buff16;
}

int Utils::ReadUtf8( const char* buffer, int* code )
{
	if ( !( ( ( unsigned char ) *buffer ) & 0x80 ) )
	{
		*code = *buffer;
		return 1;
	}
	else if ( !( ( ( unsigned char ) *buffer ) & 0x20 ) )
	{
		*code =
			( ( ( ( unsigned char ) buffer[ 0 ] ) & 0x1f ) << 6 ) +
			( ( ( ( unsigned char ) buffer[ 1 ] ) & 0x3f ) );
		return 2;
	}
	else if ( !( ( ( unsigned char ) *buffer ) & 0x10 ) )
	{
		*code =
			( ( ( ( unsigned char ) buffer[ 0 ] ) & 0x0f ) << 12 ) +
			( ( ( ( unsigned char ) buffer[ 1 ] ) & 0x3f ) << 6 ) +
			( ( ( ( unsigned char ) buffer[ 2 ] ) & 0x3f ) );
		return 3;
	}
	else if ( !( ( ( unsigned char ) *buffer ) & 0x08 ) )
	{
		*code =
			( ( ( ( unsigned char ) buffer[ 0 ] ) & 0x07 ) << 18 ) +
			( ( ( ( unsigned char ) buffer[ 1 ] ) & 0x3f ) << 12 ) +
			( ( ( ( unsigned char ) buffer[ 2 ] ) & 0x3f ) << 6 ) +
			( ( ( ( unsigned char ) buffer[ 3 ] ) & 0x3f ) );
		return 4;
	}

	/* invalid string */
	*code = 0;
	return 0;
}
