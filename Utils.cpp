#include "pch.h"
#include "Utils.h"

using namespace libeburc;

Utils::Utils()
{
}

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