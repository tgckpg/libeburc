#include "pch.h"
#include "EBAlternationCache.h"

using namespace libeburc;

EBAlternationCache::EBAlternationCache()
{
	character_number = -1;
	text = ref new Array<byte>( EB_MAX_ALTERNATION_TEXT_LENGTH + 1 );
}
