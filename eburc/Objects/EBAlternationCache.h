#pragma once
#include <pch.h>
#include <eburc/defs.h>

using namespace Platform;

namespace libeburc
{
	ref class EBAlternationCache sealed
	{
	internal:
		/*
		 * Character number.
		 */
		int character_number;

		/*
		 * Alternation string for `char_no'.
		 */
		Array<byte>^ text;

		EBAlternationCache();
	public:
	};
}
