#pragma once
#include <defs.h>
#include <EBHook.h>

namespace libeburc
{
	public ref class EBHookSet sealed
	{
	internal:
		EBHook^ hooks[ EB_NUMBER_OF_HOOKS ];
		EBHookSet();
	public:
	};
}