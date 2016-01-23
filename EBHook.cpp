#include "pch.h"
#include "EBHook.h"

using namespace libeburc;
using namespace Platform::Collections;

EBHook::EBHook( EBHookCode HookCode )
{
	code = HookCode;
	function = NULL;
}

EBHook::EBHook( HookAction^ Action, EBHookCode HookCode )
{
	code = HookCode;
	action = Action;
}

void EBHook::tryFunc(
	EBSubbook^ book, EBAppendixSubbook^ appendix
	, void *container, EBHookCode hook_code
	, int argc, const unsigned int *argv )
{
	if ( function ) function( book, appendix, container, hook_code, argc, argv );
	if ( action ) action( book );
}