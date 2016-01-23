#pragma once

#include <pch.h>
#include <defs.h>

using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace libeburc
{
	ref class EBBook;
	ref class EBSubbook;
	ref class EBAppendixSubbook;

	public delegate void HookAction( EBSubbook^ book );
	public ref class EBHook sealed
	{
	internal:
		/*
		 * Hook code.
		 */
		EBHookCode code;

		/*
		 * ABI Action call
		 */
		void ActionFunc(
			EBSubbook^ book, EBAppendixSubbook^ appendix
			, void *container, EBHookCode hook_code
			, int argc, const unsigned int *argv );

		void tryFunc(
			EBSubbook^ book, EBAppendixSubbook^ appendix
			, void *container, EBHookCode hook_code
			, int argc, const unsigned int *argv );
		/*
		 * Hook function for the hook code `code'.
		 */
		void ( *function )(
			EBSubbook^ book, EBAppendixSubbook^ appendix
			, void *container, EBHookCode hook_code
			, int argc, const unsigned int *argv );

		HookAction^ action;

		EBHook( EBHookCode HookCode );

	public:
		EBHook( HookAction^ Action, EBHookCode HookCode );
	};
}
