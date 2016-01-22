#include "pch.h"
#include "EBAppendix.h"
#include "EBAppendixSubbook.h"

using namespace libeburc;

EBAppendix::EBAppendix() { }

EBAlternationCache^ EBAppendix::AcquireWideCache( int i )
{
	if ( wide_cache[ i ] == nullptr )
		wide_cache[ i ] = ref new EBAlternationCache();
	return wide_cache[ i ];
}

EBAlternationCache^ EBAppendix::AcquireNarrowCache( int i )
{
	if ( narrow_cache[ i ] == nullptr )
		narrow_cache[ i ] = ref new EBAlternationCache();
	return narrow_cache[ i ];
}