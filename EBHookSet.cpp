#include "pch.h"
#include "EBHookSet.h"

using namespace libeburc;

EBHookSet::EBHookSet()
{
	for ( int i = 0; i < EB_NUMBER_OF_HOOKS; i++ )
	{
		hooks[ i ]->code = ( EBHookCode ) i;
		hooks[ i ]->function = NULL;
	}

	BindDefaultHooks();
}

