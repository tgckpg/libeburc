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