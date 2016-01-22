#pragma once

#include <defs.h>

namespace libeburc
{
	ref class EBBook;
	ref class EBSubbook;
	ref class EBAppendixSubbook;

	public ref class EBHook sealed
	{
	internal:
		/*
		 * Hook code.
		 */
		EBHookCode code;

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

		EBHook();
	public:
	};
}
