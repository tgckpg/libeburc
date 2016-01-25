#include "pch.h"
#include "eburc/Appendix/EBAppendix.h"
#include "eburc/Appendix/EBAppendixSubbook.h"

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

IIterable<EBAppendixSubbook^>^ EBAppendix::Subbooks::get()
{
	return subbooks->GetView();
}