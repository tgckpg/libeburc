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

byte* Utils::EucJP2Utf16( const char * c )
{
	LPCSTR pBuff = ( LPCSTR ) c;
	size_t Size = MultiByteToWideChar( CP_EUCJP, 0, pBuff, -1, NULL, 0 );

	byte* buff16 = new byte[ Size * 2 + 2 ];
	MultiByteToWideChar( CP_EUCJP, 0, pBuff, -1, ( LPWSTR ) buff16, Size );

	return buff16;
}

byte* Utils::Utf82EucJP( const wchar_t * c )
{
	LPWSTR pBuff = ( LPWSTR ) c;
	size_t Size = WideCharToMultiByte( CP_EUCJP, 0, pBuff, -1, NULL, 0, NULL, NULL );

	byte* buffjp = new byte[ Size * 2 + 2 ];
	WideCharToMultiByte( CP_EUCJP, 0, pBuff, -1, ( LPSTR ) buffjp, Size, NULL, NULL );

	return buffjp;
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

unsigned Utils::eb_bcd2( const char *stream )
{
	unsigned value;
	const unsigned char *s = ( const unsigned char * ) stream;

	value = ( ( *( s ) >> 4 ) & 0x0f ) * 1000;
	value += ( ( *( s ) ) & 0x0f ) * 100;
	value += ( ( *( s + 1 ) >> 4 ) & 0x0f ) * 10;
	value += ( ( *( s + 1 ) ) & 0x0f );

	return value;
}

unsigned Utils::eb_bcd4( const char *stream )
{
	unsigned value;
	const unsigned char *s = ( const unsigned char * ) stream;

	value = ( ( *( s ) >> 4 ) & 0x0f ) * 10000000;
	value += ( ( *( s ) ) & 0x0f ) * 1000000;
	value += ( ( *( s + 1 ) >> 4 ) & 0x0f ) * 100000;
	value += ( ( *( s + 1 ) ) & 0x0f ) * 10000;
	value += ( ( *( s + 2 ) >> 4 ) & 0x0f ) * 1000;
	value += ( ( *( s + 2 ) ) & 0x0f ) * 100;
	value += ( ( *( s + 3 ) >> 4 ) & 0x0f ) * 10;
	value += ( ( *( s + 3 ) ) & 0x0f );

	return value;
}

unsigned Utils::eb_bcd6( const char *stream )
{
	unsigned value;
	const unsigned char *s = ( const unsigned char * ) stream;

	value = ( ( *( s + 1 ) ) & 0x0f );
	value += ( ( *( s + 2 ) >> 4 ) & 0x0f ) * 10;
	value += ( ( *( s + 2 ) ) & 0x0f ) * 100;
	value += ( ( *( s + 3 ) >> 4 ) & 0x0f ) * 1000;
	value += ( ( *( s + 3 ) ) & 0x0f ) * 10000;
	value += ( ( *( s + 4 ) >> 4 ) & 0x0f ) * 100000;
	value += ( ( *( s + 4 ) ) & 0x0f ) * 1000000;
	value += ( ( *( s + 5 ) >> 4 ) & 0x0f ) * 10000000;
	value += ( ( *( s + 5 ) ) & 0x0f ) * 100000000;

	return value;
}
