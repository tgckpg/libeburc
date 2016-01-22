#include "pch.h"
#include "EBHook.h"

using namespace libeburc;

EBHook::EBHook( EBHookCode HookCode )
{
	code = HookCode;
	function = NULL;
}

void EBHook::tryFunc(
	EBSubbook^ book, EBAppendixSubbook^ appendix
	, void *container, EBHookCode hook_code
	, int argc, const unsigned int *argv )
{
	if ( function ) function( book, appendix, container, hook_code, argc, argv );
}
