#pragma once

#include <defs.h>

namespace libeburc
{
	ref class EBBook;
	ref class EBAppendix;

	public ref class EBHook sealed
	{
	internal:
		/*
		 * Hook code.
		 */
		EBHookCode code;

		/*
		 * Hook function for the hook code `code'.
		 */
		void *function(
			EBSubbook^ book, EBAppendix^ appendix
			, void *container, EBHookCode hook_code
			, int argc, const unsigned int *argv );

		EBHook();
	public:
	};
}
