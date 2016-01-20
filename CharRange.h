#pragma once
#include <pch.h>

namespace libeburc
{
	public ref class CharRange sealed
	{
		int start;
		int end;

	internal:
		CharRange( int start, int end );

	public:
		property int Start { int get() { return start; } }
		property int End { int get() { return end; } }
	};
}
