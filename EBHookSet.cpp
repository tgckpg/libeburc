#include "pch.h"
#include "EBHookSet.h"

using namespace libeburc;

EBHookSet::EBHookSet()
{
	for ( int i = 0; i < EB_NUMBER_OF_HOOKS; i++ )
	{
		hooks[ i ] = ref new EBHook( ( EBHookCode ) i );
	}

	BindDefaultHooks();
}

EBHookSet::EBHookSet( IIterable<EBHook^>^ Hooks )
{
	for_each( begin( Hooks ), end( Hooks ), [ = ](EBHook^ Hook)
	{
		hooks[ ( int ) Hook->code ] = Hook;
	} );
}